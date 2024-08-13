/*
Copyright (©) 2022-2024  Frosty515

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "kmalloc.hpp"
#include "newdelete.hpp"

#include <assert.h>
#include <spinlock.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <util.h>

#include <Memory/PageManager.hpp>
#include <Memory/PagingUtil.hpp>
#include <Memory/PhysicalPageFrameAllocator.hpp>

#include <HAL/hal.hpp>

#include <sanitiser/asan.h>

struct BlockHeader {
    size_t size : 61;
    size_t isFree : 1;
    size_t isStartOfChunk : 1;
    size_t isEndOfChunk : 1;
    BlockHeader* next;
} __attribute__((packed));

/*
Each chunk of memory allocated by the allocator will have a header,
which will contain the size of the chunk, whether it is free or not, and a pointer to the next chunk.

The allocator will keep a pointer to the first chunk in the free list.

When a chunk is allocated, the allocator will search the free list for a chunk that is large enough.
If it finds one, it will remove it from the free list and return it.

When a chunk is freed, the allocator will add it to the free list.

Each chunk contains multiple blocks. Each block can vary in size, but it must be 16 byte aligned.
*/

#define MIN_SIZE 16

struct HeapExpander {
    void* (*Allocate)(size_t size);
    void (*Free)(void* ptr, size_t size);
};

class HeapAllocator {
public:
    __attribute__((no_sanitize("address","kernel-address"))) HeapAllocator();
    __attribute__((no_sanitize("address","kernel-address"))) HeapAllocator(HeapExpander expander, size_t size);

    __attribute__((no_sanitize("address","kernel-address"))) void* allocate(size_t size);
    __attribute__((no_sanitize("address","kernel-address"))) void free(void* ptr);

    size_t getFreeMem() const;
    size_t getUsedMem() const;
    size_t getMetadataMem() const;
    size_t getTotalMem() const;

    void lock();
    void unlock();

    void PrintBlocks(fd_t fd);

    __attribute__((no_sanitize("address","kernel-address"))) void* getHeapStart() const { return m_heapStart; }
    __attribute__((no_sanitize("address","kernel-address"))) void* getHeapEnd() const { return m_heapEnd; }

    __attribute__((no_sanitize("address","kernel-address"))) void Verify();
    __attribute__((no_sanitize("address","kernel-address"))) void VerifyHeader(BlockHeader* header, bool free);
    __attribute__((no_sanitize("address","kernel-address"))) void VerifyRegion(void* ptr, bool free);

private:
    __attribute__((no_sanitize("address","kernel-address"))) void AddToFreeList(BlockHeader* header);
    void CheckForDeletion(); // will check if any chunks in the free list can be deleted, and delete them if they can

private:
    HeapExpander m_expander;
    BlockHeader* m_freeList;
    size_t m_freeMem;
    size_t m_usedMem;
    size_t m_MetadataMem;
    spinlock_t m_lock;

    void* m_heapStart;
    void* m_heapEnd;
};

__attribute__((no_sanitize("address","kernel-address"))) HeapAllocator::HeapAllocator() : m_expander(nullptr, nullptr), m_freeList(nullptr), m_freeMem(0), m_usedMem(0), m_MetadataMem(0) {
    spinlock_init(&m_lock);
}

__attribute__((no_sanitize("address","kernel-address"))) HeapAllocator::HeapAllocator(HeapExpander expander, size_t size) : m_expander(expander), m_freeList(nullptr), m_freeMem(0), m_usedMem(0), m_MetadataMem(0) {
    spinlock_init(&m_lock);
    size_t numBytes = ALIGN_UP((size + sizeof(BlockHeader)), PAGE_SIZE);
    void* ptr = m_expander.Allocate(numBytes);
    BlockHeader* header = (BlockHeader*)ptr;
    m_MetadataMem += sizeof(BlockHeader);
    header->size = numBytes - sizeof(BlockHeader);
    header->isFree = true;
    header->isStartOfChunk = true;
    header->isEndOfChunk = true;
    m_freeMem += header->size;
    m_freeList = header;
    m_heapStart = ptr;
    m_heapEnd = (void*)((uint64_t)ptr + numBytes);
}

__attribute__((no_sanitize("address","kernel-address"))) void* HeapAllocator::allocate(size_t size) {
    size = ALIGN_UP(size, MIN_SIZE);

    // Search the free list for a chunk that is large enough
    BlockHeader* prev = nullptr;
    BlockHeader* curr = m_freeList;
    while (curr != nullptr) {
        if (curr->size >= size) {
            // Found a chunk that is large enough
            if (prev == nullptr) // This is the first chunk in the free list
                m_freeList = curr->next;
            else // This is not the first chunk in the free list
                prev->next = curr->next;
            curr->isFree = false;
            if (curr->size > (size + sizeof(BlockHeader))) {
                // Split the chunk into two blocks
                BlockHeader* next = (BlockHeader*)((uint64_t)curr + sizeof(BlockHeader) + size);
                next->size = curr->size - size - sizeof(BlockHeader);
                next->isFree = true;
                curr->size = size;
                if (curr->isEndOfChunk) {
                    curr->isEndOfChunk = false;
                    next->isEndOfChunk = true;
                }
                else
                    next->isEndOfChunk = false;
                next->isStartOfChunk = false;
                m_freeMem -= sizeof(BlockHeader);
                AddToFreeList(next);
                m_MetadataMem += sizeof(BlockHeader);
            }
            m_usedMem += size;
            m_freeMem -= size;
            return (void*)((uint64_t)curr + sizeof(BlockHeader));
        }
        prev = curr;
        curr = curr->next;
    }

    // Didn't find a chunk that is large enough, so we attempt to expand the heap
    size_t chunkSize = ALIGN_UP((size + sizeof(BlockHeader)), PAGE_SIZE);
    void* ptr = m_expander.Allocate(chunkSize);
    if (ptr != nullptr) {
        BlockHeader* header = (BlockHeader*)ptr;
        header->size = size;
        header->isFree = false;
        header->isStartOfChunk = true;
        m_usedMem += size;
        m_MetadataMem += sizeof(BlockHeader);
        if (chunkSize > (size + 2 * sizeof(BlockHeader))) {
            BlockHeader* next = (BlockHeader*)((uint64_t)ptr + sizeof(BlockHeader) + size);
            header->isEndOfChunk = false;
            next->size = chunkSize - size - 2 * sizeof(BlockHeader); // block header of this and allocated block
            next->isFree = true;
            next->isStartOfChunk = false;
            next->isEndOfChunk = true;
            m_freeMem += next->size;
            m_MetadataMem += sizeof(BlockHeader);
            // print info about next
            AddToFreeList(next);
        }
        else {
            header->size = chunkSize - sizeof(BlockHeader);
            header->isEndOfChunk = true;
        }
        m_heapEnd = (void*)((uint64_t)ptr + chunkSize); // FIXME: doesn't work
        return (void*)((uint64_t)ptr + sizeof(BlockHeader));
    }
    PANIC("kmalloc: out of memory");
    return nullptr;
}

__attribute__((no_sanitize("address","kernel-address"))) void HeapAllocator::free(void* ptr) {
    BlockHeader* header = (BlockHeader*)((uint64_t)ptr - sizeof(BlockHeader));
    assert(header->size > 0);
    assert(!header->isFree);
    header->isFree = true;
    if (header->isStartOfChunk && header->isEndOfChunk) {
        // this is the only block in the chunk
        m_usedMem -= header->size;
        m_MetadataMem -= sizeof(BlockHeader);
        m_expander.Free(header, header->size + sizeof(BlockHeader));
        return;
    }
    m_usedMem -= header->size;
    m_freeMem += header->size;
    AddToFreeList(header);
}

void* HeapExpand(size_t size);

__attribute__((no_sanitize("address","kernel-address"))) void HeapAllocator::AddToFreeList(BlockHeader* header) {
    // VerifyHeader(header, true);
    if (m_freeList == nullptr) {
        m_freeList = header;
        header->next = nullptr;
        return;
    }
    // Verify();
    // search through the list to find the correct place to insert the chunk
    BlockHeader* prev_prev = nullptr;
    BlockHeader* prev = nullptr;
    BlockHeader* curr = m_freeList;
    while (curr != nullptr) {
        if (header > prev && header < curr) {
            // found the correct place to insert the chunk
            if (prev == nullptr) {
                // this is the first chunk in the free list
                m_freeList = header;
                // see if we can merge with next. this will only happen if the next chunk is free
                if ((uint64_t)header + sizeof(BlockHeader) + header->size == (uint64_t)curr && curr->isFree && !curr->isStartOfChunk && !header->isEndOfChunk) {
                    header->size += sizeof(BlockHeader) + curr->size;
                    if (curr->isEndOfChunk)
                        header->isEndOfChunk = true;
                    header->next = curr->next;
                    m_freeMem += sizeof(BlockHeader);
                    m_MetadataMem -= sizeof(BlockHeader);
                } 
                else
                    header->next = curr;
                if (header->isStartOfChunk && header->isEndOfChunk) {
                    // this is the only block in the chunk
                    m_freeList = header->next;
                    m_usedMem -= header->size;
                    m_MetadataMem -= sizeof(BlockHeader);
                    m_expander.Free(header, header->size + sizeof(BlockHeader));
                }
            }
            else {
                // this is not the first chunk in the free list

                // see if we can merge with prev. this will only happen if the prev chunk is free
                if ((uint64_t)prev + sizeof(BlockHeader) + prev->size == (uint64_t)header && prev->isFree && !header->isStartOfChunk && !prev->isEndOfChunk) {
                    prev->size += sizeof(BlockHeader) + header->size;
                    prev->next = curr;
                    if (header->isEndOfChunk)
                        prev->isEndOfChunk = true;
                    header = prev;
                    m_freeMem += sizeof(BlockHeader);
                    m_MetadataMem -= sizeof(BlockHeader);
                }
                else
                    prev->next = header;
                // see if we can merge with next. this will only happen if the next chunk is free
                if ((uint64_t)header + sizeof(BlockHeader) + header->size == (uint64_t)curr && curr->isFree && !curr->isStartOfChunk && !header->isEndOfChunk) {
                    header->size += sizeof(BlockHeader) + curr->size;
                    header->next = curr->next;
                    if (curr->isEndOfChunk)
                        header->isEndOfChunk = true;
                    m_freeMem += sizeof(BlockHeader);
                    m_MetadataMem -= sizeof(BlockHeader);
                }
                else
                    header->next = curr;
                if (header->isStartOfChunk && header->isEndOfChunk) {
                    // this is the only block in the chunk
                    if (header == prev) {
                        if (prev_prev != nullptr)
                            prev_prev->next = header->next;
                        else
                            m_freeList = header->next;
                    }
                    else
                        prev->next = header->next;
                    m_usedMem -= header->size;
                    m_MetadataMem -= sizeof(BlockHeader);
                    m_expander.Free(header, header->size + sizeof(BlockHeader));
                }
            }
            return;
        }
        prev_prev = prev;
        prev = curr;
        curr = curr->next;
    }

    // didn't find the correct place to insert the chunk, so we just insert it at the end
    prev->next = header;
    header->next = nullptr;
    // see if we can merge with prev. this will only happen if the prev chunk is free
    if ((uint64_t)prev + sizeof(BlockHeader) + prev->size == (uint64_t)header && prev->isFree && !header->isStartOfChunk && !prev->isEndOfChunk) {
        prev->size += sizeof(BlockHeader) + header->size;
        prev->next = nullptr;
        if (header->isEndOfChunk)
            prev->isEndOfChunk = true;
        header = prev;
        m_freeMem += sizeof(BlockHeader);
        m_MetadataMem -= sizeof(BlockHeader);
    }
    if (header->isStartOfChunk && header->isEndOfChunk) {
        // this is the only block in the chunk
        if (prev != nullptr)
            prev->next = header->next;
        else
            m_freeList = header->next;
        m_usedMem -= header->size;
        m_MetadataMem -= sizeof(BlockHeader);
        m_expander.Free(header, header->size + sizeof(BlockHeader));
    }
}

size_t HeapAllocator::getFreeMem() const {
    return m_freeMem;
}

size_t HeapAllocator::getUsedMem() const {
    return m_usedMem;
}

size_t HeapAllocator::getMetadataMem() const {
    return m_MetadataMem;
}

size_t HeapAllocator::getTotalMem() const {
    return m_usedMem + m_freeMem + m_MetadataMem;
}

void HeapAllocator::lock() {
    spinlock_acquire(&m_lock);
}
void HeapAllocator::unlock() {
    spinlock_release(&m_lock);
}

__attribute__((no_sanitize("address","kernel-address"))) void HeapAllocator::PrintBlocks(fd_t fd) {
    BlockHeader* curr = m_freeList;
    while (isInKernelSpace(curr, sizeof(BlockHeader))) {
        fprintf(fd, "Block: %lp, base = %lp, size = %lx, isFree = %d, isStartOfChunk = %d, isEndOfChunk = %d, next = %lp\n", curr, (void*)((uint64_t)curr + sizeof(BlockHeader)), curr->size, curr->isFree, curr->isStartOfChunk, curr->isEndOfChunk, curr->next);
        curr = curr->next;
    }
}

__attribute__((no_sanitize("address","kernel-address"))) void HeapAllocator::Verify() {
    BlockHeader* curr = m_freeList;
    while (curr != nullptr) {
        assert(isInKernelSpace(curr, sizeof(BlockHeader)));
        assert(curr->isFree);
        assert(curr->size > 0);
        assert(isInKernelSpace(curr, sizeof(BlockHeader) + curr->size));
        if (curr->isStartOfChunk)
            assert(((uint64_t)curr % PAGE_SIZE) == 0);
        if (curr->isEndOfChunk)
            assert((((uint64_t)curr + curr->size + sizeof(BlockHeader)) % PAGE_SIZE) == 0);
        curr = curr->next;
    }
}

__attribute__((no_sanitize("address","kernel-address"))) void HeapAllocator::VerifyHeader(BlockHeader* header, bool free) {
    assert(isInKernelSpace(header, sizeof(BlockHeader)));
    assert(header->isFree == free);
    assert(header->size > 0);
    assert(isInKernelSpace(header, sizeof(BlockHeader) + header->size));
    if (header->isStartOfChunk)
        assert(((uint64_t)header % PAGE_SIZE) == 0);
    if (header->isEndOfChunk)
        assert((((uint64_t)header + header->size + sizeof(BlockHeader)) % PAGE_SIZE) == 0);
}

__attribute__((no_sanitize("address","kernel-address"))) void HeapAllocator::VerifyRegion(void* ptr, bool free) {
    VerifyHeader((BlockHeader*)((uint64_t)ptr - sizeof(BlockHeader)), free);
}


HeapAllocator g_VMMAllocator;

bool g_kmalloc_vmm_initialised;


__attribute__((no_sanitize("address","kernel-address"))) void* VMMHeapExpand(size_t size) {
    // g_VMMAllocator.Verify();
#ifdef _FROSTYOS_BUILD_TARGET_IS_KERNEL
    void* mem = to_HHDM(g_PPFA->AllocatePages(DIV_ROUNDUP(size, PAGE_SIZE)));
#elif defined(_FROSTYOS_BUILD_TARGET_IS_USERLAND)
    void* mem = __user_raw_mmap(nullptr, size, PagePermissions::READ_WRITE);
#endif
    if (mem == nullptr)
        return nullptr;
    // g_VMMAllocator.Verify();
    memset_nokasan(mem, 0, ALIGN_UP(size, PAGE_SIZE));
    // g_VMMAllocator.Verify();
    return mem;
}

__attribute__((no_sanitize("address","kernel-address"))) void VMMHeapShrink(void* ptr, size_t size) {
#ifdef _FROSTYOS_BUILD_TARGET_IS_KERNEL
    void* phys_addr = hhdm_to_phys(ptr);
    g_PPFA->FreePages(phys_addr, DIV_ROUNDUP(size, PAGE_SIZE));
#elif defined(_FROSTYOS_BUILD_TARGET_IS_USERLAND)
    __user_munmap(ptr, size);
#endif
}

__attribute__((no_sanitize("address","kernel-address"))) void kmalloc_vmm_init() {
    g_kmalloc_vmm_initialised = false;

    g_VMMAllocator = HeapAllocator({VMMHeapExpand, VMMHeapShrink}, PAGE_SIZE);

    g_kmalloc_vmm_initialised = true;
}

__attribute__((no_sanitize("address","kernel-address"))) bool IsKmallocVMMInitialised() {
    return g_kmalloc_vmm_initialised;
}

__attribute__((no_sanitize("address","kernel-address"))) void* kmalloc_vmm(size_t size) {
    if (!g_kmalloc_vmm_initialised)
        return nullptr;
    size = ALIGN_UP(size, 8);
#if _FROSTYOS_ENABLE_KASAN
    size += 256;
#endif
    g_VMMAllocator.lock();
    void* mem = g_VMMAllocator.allocate(ALIGN_UP(size, MIN_SIZE));
#if _FROSTYOS_ENABLE_KASAN
    memset_nokasan((void*)((uint64_t)mem + size - 256), ASANPoisonValues[ASANPoison_Allocated], 256);
    if (memcmp_b(mem, ASANPoisonValues[ASANPoison_Freed], 8))
        memset_nokasan(mem, 0, size - 256);
#endif
    g_VMMAllocator.unlock();
    return mem;
}

__attribute__((no_sanitize("address","kernel-address"))) void* kcalloc_vmm(size_t num, size_t size) {
    if (!g_kmalloc_vmm_initialised)
        return nullptr;
    size = ALIGN_UP(size * num, 8);
#if _FROSTYOS_ENABLE_KASAN
    size += 256;
#endif
    g_VMMAllocator.lock();
    void* mem = g_VMMAllocator.allocate(ALIGN_UP(size, MIN_SIZE));
#if _FROSTYOS_ENABLE_KASAN
    memset_nokasan((void*)((uint64_t)mem + size - 256), ASANPoisonValues[ASANPoison_Allocated], 256);
    memset_nokasan(mem, 0, size - 256);
#else
    memset(mem, 0, size);
#endif
    g_VMMAllocator.unlock();
    return mem;
}

__attribute__((no_sanitize("address","kernel-address"))) void* krealloc_vmm(void* ptr, size_t size) {
    if (size == 0) {
        kfree_vmm(ptr);
        return nullptr;
    }
    if (ptr == nullptr)
        return kmalloc_vmm(size);
    g_VMMAllocator.lock();
    BlockHeader* header = (BlockHeader*)((uint64_t)ptr - sizeof(BlockHeader));
    size_t old_size = header->size;
    void* ptr2 = g_VMMAllocator.allocate(ALIGN_UP(size, MIN_SIZE));
    if (ptr2 == nullptr)
        return nullptr;
    memcpy_nokasan(ptr2, ptr, old_size);
    g_VMMAllocator.free(ptr);
    g_VMMAllocator.unlock();
    return ptr2;
}

__attribute__((no_sanitize("address","kernel-address"))) void kfree_vmm(void* addr) {
    if (addr == nullptr)
        return;
    g_VMMAllocator.lock();
#if _FROSTYOS_ENABLE_KASAN
    BlockHeader* header = (BlockHeader*)((uint64_t)addr - sizeof(BlockHeader));
    size_t size = header->size;
#endif
    g_VMMAllocator.free(addr);
#if _FROSTYOS_ENABLE_KASAN
    memset_nokasan(addr, ASANPoisonValues[ASANPoison_Freed], size);
#endif
    g_VMMAllocator.unlock();
}




HeapAllocator g_heapAllocator;

bool g_kmalloc_initialised;

__attribute__((no_sanitize("address","kernel-address"))) void* HeapExpand(size_t size) {
    // g_heapAllocator.Verify();
    void* mem = g_KPM->AllocatePages(DIV_ROUNDUP(size, PAGE_SIZE));
    if (mem == nullptr)
        return nullptr;
    // g_heapAllocator.Verify();
    memset(mem, 0, ALIGN_UP(size, PAGE_SIZE));
    // g_heapAllocator.Verify();
    return mem;
}

__attribute__((no_sanitize("address","kernel-address"))) void HeapShrink(void* ptr, size_t size) {
    assert((size % PAGE_SIZE) == 0);
    g_KPM->FreePages(ptr);
    // g_heapAllocator.Verify();
}

__attribute__((no_sanitize("address","kernel-address"))) void kmalloc_init() {
    g_kmalloc_initialised = false;
    
    g_heapAllocator = HeapAllocator({HeapExpand, HeapShrink}, PAGE_SIZE);

    g_kmalloc_initialised = true;
    NewDeleteInit();
}
 
__attribute__((no_sanitize("address","kernel-address"))) bool IsKmallocInitialised() {
    return g_kmalloc_initialised;
}

__attribute__((no_sanitize("address","kernel-address"))) size_t kmalloc_SizeInHeap(void *ptr, size_t size) {
    return 0; // FIXME: heap start and end don't work yet
    if (ptr == nullptr)
        return 0;
    if (!(ptr >= g_heapAllocator.getHeapStart() && ptr < g_heapAllocator.getHeapEnd()))
        return 0;
    if (((uint64_t)ptr + size) <= (uint64_t)g_heapAllocator.getHeapEnd())
        return size;
    else
        return (uint64_t)g_heapAllocator.getHeapEnd() - (uint64_t)ptr;
}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void* kcalloc(size_t num, size_t size) {
    if (!g_kmalloc_initialised)
        return nullptr;
    size = ALIGN_UP((size * num), MIN_SIZE);
#if _FROSTYOS_ENABLE_KASAN
    size += 256;
#endif
    g_heapAllocator.lock();
    void* mem = g_heapAllocator.allocate(size);
#if _FROSTYOS_ENABLE_KASAN
    memset_nokasan((void*)((uint64_t)mem + size - 256), ASANPoisonValues[ASANPoison_Allocated], 256);
    memset_nokasan(mem, 0, size - 256);
#else
    memset(mem, 0, size);
#endif
    // g_heapAllocator.Verify();
    // g_heapAllocator.VerifyRegion(mem, false);
    g_heapAllocator.unlock();
    return mem;
}

extern void x86_64_walk_stack_frames(void* RBP);

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void kfree(void* addr) {
    if (addr == nullptr)
        return;
    g_heapAllocator.lock();
#if _FROSTYOS_ENABLE_KASAN
    BlockHeader* header = (BlockHeader*)((uint64_t)addr - sizeof(BlockHeader));
    size_t size = header->size;
#endif
    g_heapAllocator.free(addr);
#if _FROSTYOS_ENABLE_KASAN
    memset_nokasan(addr, ASANPoisonValues[ASANPoison_Freed], size);
#endif
    // g_heapAllocator.Verify();
    g_heapAllocator.unlock();
}

extern "C" __attribute__((no_sanitize("address","kernel-address"))) void* kmalloc(size_t size) {
    if (size == 0 || !g_kmalloc_initialised)
        return nullptr;
#if _FROSTYOS_ENABLE_KASAN
    size += 256;
#endif
    g_heapAllocator.lock();
    // g_heapAllocator.Verify();
    void* mem = g_heapAllocator.allocate(ALIGN_UP(size, MIN_SIZE));
#if _FROSTYOS_ENABLE_KASAN
    memset_nokasan((void*)((uint64_t)mem + size - 256), ASANPoisonValues[ASANPoison_Allocated], 256);
    if (memcmp_b(mem, ASANPoisonValues[ASANPoison_Freed], 8))
        memset_nokasan(mem, 0, size - 256);
#endif
    // g_heapAllocator.Verify();
    // g_heapAllocator.VerifyRegion(mem, false);
    g_heapAllocator.unlock();
    return mem;
}

extern "C" void* __attribute__((no_sanitize("address","kernel-address"))) krealloc(void* ptr, size_t size) {
    if (size == 0) {
        kfree(ptr);
        return nullptr;
    }
    if (ptr == nullptr)
        return kmalloc(size);
    g_heapAllocator.lock();
    BlockHeader* header = (BlockHeader*)((uint64_t)ptr - sizeof(BlockHeader));
    size_t old_size = header->size;
    void* ptr2 = g_heapAllocator.allocate(ALIGN_UP(size, MIN_SIZE));
    if (ptr2 == nullptr)
        return nullptr;
    memcpy_nokasan(ptr2, ptr, old_size);
    g_heapAllocator.free(ptr);
    // g_heapAllocator.Verify();
    g_heapAllocator.unlock();
    return ptr2;
}


bool g_kmalloc_eternal_initialised = false;

void* g_kmalloc_eternal_mem = nullptr;
size_t g_kmalloc_eternal_free_mem = 0;
size_t g_kmalloc_eternal_used_mem = 0;

spinlock_new(g_kmalloc_eternal_lock);

void kmalloc_eternal_init() {
    g_kmalloc_eternal_initialised = false;

    void* pages = g_KPM->AllocatePages(512); // ~2MiB
    if (pages == nullptr)
        return;
    g_kmalloc_eternal_mem = pages;
    g_kmalloc_eternal_free_mem = 512 * PAGE_SIZE;
    g_kmalloc_eternal_used_mem = 0;

    spinlock_init(&g_kmalloc_eternal_lock);

    g_kmalloc_eternal_initialised = true;
}

extern "C" void* kmalloc_eternal(size_t size) {
    size = ALIGN_UP(size, 8);

    spinlock_acquire(&g_kmalloc_eternal_lock);

    if (!g_kmalloc_eternal_initialised || size > g_kmalloc_eternal_free_mem)
        return nullptr;

    void* mem = g_kmalloc_eternal_mem;
    g_kmalloc_eternal_used_mem += size;
    g_kmalloc_eternal_free_mem -= size;
    g_kmalloc_eternal_mem = (void*)((uint64_t)g_kmalloc_eternal_mem + size);

    spinlock_release(&g_kmalloc_eternal_lock);

    return mem;
}

extern "C" void* kcalloc_eternal(size_t num, size_t size) {
    size = ALIGN_UP(size * num, 8);

    spinlock_acquire(&g_kmalloc_eternal_lock);

    if (!g_kmalloc_eternal_initialised || size > g_kmalloc_eternal_free_mem)
        return nullptr;

    void* mem = g_kmalloc_eternal_mem;
    g_kmalloc_eternal_used_mem += size;
    g_kmalloc_eternal_free_mem -= size;
    g_kmalloc_eternal_mem = (void*)((uint64_t)g_kmalloc_eternal_mem + size);

    spinlock_release(&g_kmalloc_eternal_lock);

    fast_memset(mem, 0, size / 8);

    return mem;
}
