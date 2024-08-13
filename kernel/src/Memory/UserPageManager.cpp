#include "arch/x86_64/Memory/PagingUtil.hpp"
#include "newdelete.hpp"
#include "PageObject.hpp"
#include "PageManager.hpp"
#include "util.h"

#include <spinlock.h>
#include <string.h>

#define MAP_GROWSDOWN	0x00100		/* Stack-like segment.  */
#define MAP_DENYWRITE	0x00800		/* ETXTBSY.  */
#define MAP_EXECUTABLE	0x01000		/* Mark it as an executable.  */
#define MAP_LOCKED	0x02000		/* Lock the mapping.  */
#define MAP_NORESERVE	0x04000		/* Don't check for reservations.  */
#define MAP_POPULATE	0x08000		/* Populate (prefault) pagetables.  */
#define MAP_NONBLOCK	0x10000		/* Do not block on IO.  */
#define MAP_STACK	0x20000		/* Allocation is for a stack.  */
#define MAP_HUGETLB	0x40000		/* Create huge page mapping.  */
#define MAP_SYNC	0x80000		/* Perform synchronous page faults for the mapping.  */
#define MAP_FIXED_NOREPLACE 0x100000	/* MAP_FIXED but do not unmap underlying mapping.  */

#define PROT_READ	0x1		/* Page can be read.  */
#define PROT_WRITE	0x2		/* Page can be written.  */
#define PROT_EXEC	0x4		/* Page can be executed.  */
#define PROT_NONE	0x0		/* Page can not be accessed.  */
#define PROT_GROWSDOWN	0x01000000	/* Extend change to start of growsdown vma (mprotect only).  */
#define PROT_GROWSUP	0x02000000	/* Extend change to start of growsup vma (mprotect only).  */

/* Sharing types (must choose one and only one of these).  */
#define MAP_SHARED	0x01		/* Share changes.  */
#define MAP_PRIVATE	0x02		/* Changes are private.  */
#define MAP_SHARED_VALIDATE	0x03	/* Share changes and validate extension flags.  */
#define MAP_TYPE	0x0f		/* Mask for type of mapping.  */

/* Other flags.  */
#define MAP_FIXED	0x10		/* Interpret addr exactly.  */
#define MAP_FILE	0
#ifdef __MAP_ANONYMOUS
# define MAP_ANONYMOUS	__MAP_ANONYMOUS	/* Don't use a file.  */
#else
# define MAP_ANONYMOUS	0x20		/* Don't use a file.  */
#endif
#define MAP_ANON	MAP_ANONYMOUS

PageManager::PageManager() : m_allocated_objects(true), m_allocated_object_count(0), m_lock(0) {

}

void PageManager::InitPageManager() {
    if (!PageObjectPool_HasBeenInitialised())
        PageObjectPool_Init();
}

PageManager::~PageManager() {

}

void* PageManager::AllocatePage(PagePermissions perms, void* addr) {
    int prot = PROT_NONE;
    switch (perms) {
    case PagePermissions::READ:
        prot = PROT_READ;
        break;
    case PagePermissions::WRITE:
        prot = PROT_WRITE;
        break;
    case PagePermissions::EXECUTE:
        prot = PROT_EXEC;
        break;
    case PagePermissions::READ_WRITE:
        prot = PROT_READ | PROT_WRITE;
        break;
    case PagePermissions::READ_EXECUTE:
        prot = PROT_READ | PROT_EXEC;
        break;
    default:
        break;
    }
    int flags = MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE;
    if (addr != nullptr)
        flags |= MAP_FIXED_NOREPLACE;
    void* ret = __user_mmap(addr, PAGE_SIZE, prot, flags, -1, 0);
    if ((int64_t)ret < 0)
        return nullptr;
    spinlock_acquire(&m_lock);
    PageObject* po = CreateObject();
    if (po == nullptr) {
        __user_munmap(ret, PAGE_SIZE);
        spinlock_release(&m_lock);
        return nullptr;
    }
    po->flags |= PO_ALLOCATED | PO_INUSE;
    po->virtual_address = ret;
    po->page_count = 1;
    InsertObject(po);
    m_allocated_object_count++;
    po->perms = perms;
    spinlock_release(&m_lock);
    return ret;
}

void* PageManager::AllocatePages(uint64_t count, PagePermissions perms, void* addr) {
    int prot = PROT_NONE;
    switch (perms) {
    case PagePermissions::READ:
        prot = PROT_READ;
        break;
    case PagePermissions::WRITE:
        prot = PROT_WRITE;
        break;
    case PagePermissions::EXECUTE:
        prot = PROT_EXEC;
        break;
    case PagePermissions::READ_WRITE:
        prot = PROT_READ | PROT_WRITE;
        break;
    case PagePermissions::READ_EXECUTE:
        prot = PROT_READ | PROT_EXEC;
        break;
    default:
        break;
    }
    int flags = MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE;
    if (addr != nullptr)
        flags |= MAP_FIXED_NOREPLACE;
    void* ret = __user_mmap(addr, PAGE_SIZE * count, prot, flags, -1, 0);
    if ((int64_t)ret < 0) {
        printf("mmap failed: %ld\n", strerror(-((int64_t)ret)), -((int64_t)ret));
        return nullptr;
    }
    spinlock_acquire(&m_lock);
    PageObject* po = CreateObject();
    if (po == nullptr) {
        __user_munmap(ret, PAGE_SIZE);
        spinlock_release(&m_lock);
        puts("po is null\n");
        return nullptr;
    }
    po->flags |= PO_ALLOCATED | PO_INUSE;
    po->virtual_address = ret;
    po->page_count = count;
    InsertObject(po);
    m_allocated_object_count++;
    po->perms = perms;
    spinlock_release(&m_lock);
    return ret;
}

void* PageManager::ReservePage(PagePermissions perms, void* addr) {
    return nullptr;
}

void* PageManager::ReservePages(uint64_t count, PagePermissions perms, void* addr) {
    return nullptr; // not implemented
}

void PageManager::FreePage(void* addr) {
    FreePages(addr);
}

void PageManager::FreePages(void* addr) {
    spinlock_acquire(&m_lock);
    PageObject* obj = FindObject(addr);
    if (obj == nullptr) {
        spinlock_release(&m_lock);
        return;
    }
    RemoveObject(obj);
    __user_munmap(obj->virtual_address, obj->page_count * PAGE_SIZE);
    DeleteObject(obj);
    spinlock_release(&m_lock);
}

void PageManager::Remap(void* addr, PagePermissions perms) {
    int prot = PROT_NONE;
    switch (perms) {
    case PagePermissions::READ:
        prot = PROT_READ;
        break;
    case PagePermissions::WRITE:
        prot = PROT_WRITE;
        break;
    case PagePermissions::EXECUTE:
        prot = PROT_EXEC;
        break;
    case PagePermissions::READ_WRITE:
        prot = PROT_READ | PROT_WRITE;
        break;
    case PagePermissions::READ_EXECUTE:
        prot = PROT_READ | PROT_EXEC;
        break;
    default:
        break;
    }
    spinlock_acquire(&m_lock);
    PageObject* obj = FindObject(addr);
    if (obj == nullptr) {
        spinlock_release(&m_lock);
        return;
    }
    __user_mprotect(addr, obj->page_count * PAGE_SIZE, prot);
    spinlock_release(&m_lock);
}

void* PageManager::MapPage(void* phys, PagePermissions perms, void* addr) {
    return nullptr; // not supported
}

void* PageManager::MapPages(void* phys, uint64_t count, PagePermissions perms, void* addr) {
    return nullptr; // not supported
}

void PageManager::UnmapPage(void* addr) {
    // not supported
}

void PageManager::UnmapPages(void* addr) {
    // not supported
}

bool isInKernelSpace(void* base, size_t length) {
    // TODO: implement this properly
    (void)length;
    return base != nullptr;
}

PageManager* g_KPM;

void* __user_raw_mmap(void* addr, size_t size, PagePermissions perms) {
    int prot = PROT_NONE;
    switch (perms) {
    case PagePermissions::READ:
        prot = PROT_READ;
        break;
    case PagePermissions::WRITE:
        prot = PROT_WRITE;
        break;
    case PagePermissions::EXECUTE:
        prot = PROT_EXEC;
        break;
    case PagePermissions::READ_WRITE:
        prot = PROT_READ | PROT_WRITE;
        break;
    case PagePermissions::READ_EXECUTE:
        prot = PROT_READ | PROT_EXEC;
        break;
    default:
        break;
    }
    int flags = MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE;
    if (addr != nullptr)
        flags |= MAP_FIXED_NOREPLACE;
    return __user_mmap(addr, size, prot, flags, -1, 0);
}

void __user_raw_mprotect(void* addr, size_t size, PagePermissions perms) {
    int prot = PROT_NONE;
    switch (perms) {
    case PagePermissions::READ:
        prot = PROT_READ;
        break;
    case PagePermissions::WRITE:
        prot = PROT_WRITE;
        break;
    case PagePermissions::EXECUTE:
        prot = PROT_EXEC;
        break;
    case PagePermissions::READ_WRITE:
        prot = PROT_READ | PROT_WRITE;
        break;
    case PagePermissions::READ_EXECUTE:
        prot = PROT_READ | PROT_EXEC;
        break;
    default:
        break;
    }
    __user_mprotect(addr, size, prot);
}