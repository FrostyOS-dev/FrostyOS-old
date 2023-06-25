#ifndef _X86_64_VIRTUAL_PAGE_MGR_HPP
#define _X86_64_VIRTUAL_PAGE_MGR_HPP

#include <Memory/Memory.hpp>

#include <stdint.h>

#include <Data-structures/AVLTree.hpp>
#include <Data-structures/LinkedList.hpp>

namespace WorldOS {
    class VirtualPageManager {
    public:
        VirtualPageManager();
        ~VirtualPageManager();
        void InitVPageMgr(MemoryMapEntry** MemoryMap, uint64_t MemoryMapEntryCount, void* kernel_virt_start, size_t kernel_size, void* fb_virt, uint64_t fb_size, void* region_start, uint64_t region_length);
        void* FindFreePage();
        void* FindFreePages(uint64_t count);
        void cleanupRUTree(AVLTree::Node* node, AVLTree::Node* parent);
        void CleanupFreePagesTree();
        void ReservePage(void* addr);
        void ReservePages(void* addr, uint64_t count);
        void UnreservePage(void* addr);
        void UnreservePages(void* addr, uint64_t count);
        void* AllocatePage();
        void* AllocatePages(uint64_t count);
        void UnallocatePage(void* addr);
        void UnallocatePages(void* addr, uint64_t count);
    private:
        void LockPage(void* addr);
        void LockPages(void* addr, uint64_t count);
        void UnlockPage(void* addr);
        void UnlockPages(void* addr, uint64_t count);
        void FreePage(void* addr);
        void FreePages(void* addr, uint64_t count);
        void UnfreePage(void* addr);
        void UnfreePages(void* addr, uint64_t count);
    private:
        uint64_t m_FreePagesCount;
        AVLTree::Node* m_FreePagesSizeTree;
        uint64_t m_RegionLength;
        void* m_RegionStart;
        AVLTree::Node* m_ReservedANDUsedPages; // highest bit in extra data determines if it reserved (1) or used (0)
        uint64_t m_ReservedPagesCount;
        uint64_t m_UsedPagesCount;
    };

    extern VirtualPageManager* g_KVPM; // this is probably not the best place for this, but there are no other options
}

#endif /* _X86_64_VIRTUAL_PAGE_MGR_HPP */