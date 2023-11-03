/*
Copyright (©) 2022-2023  Frosty515

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

#ifndef _VIRTUAL_PAGE_MGR_HPP
#define _VIRTUAL_PAGE_MGR_HPP

#include "Memory.hpp"
#include "VirtualRegion.hpp"

#include <stdint.h>

#include <Data-structures/AVLTree.hpp>
#include <Data-structures/LinkedList.hpp>

class VirtualPageManager {
public:
    VirtualPageManager();
    ~VirtualPageManager();
    void InitVPageMgr(MemoryMapEntry** MemoryMap, uint64_t MemoryMapEntryCount, void* kernel_virt_start, size_t kernel_size, void* fb_virt, uint64_t fb_size, const VirtualRegion& region);
    void InitVPageMgr(const VirtualRegion& region);

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
    void* AllocatePage(void* addr);
    void* AllocatePages(void* addr, uint64_t count);
    void UnallocatePage(void* addr);
    void UnallocatePages(void* addr, uint64_t count);

    const VirtualRegion& GetVirtualRegion() const;

    bool AttemptToExpandRight(size_t new_size);

private:
    void LockPage(void* addr, bool unfree = true, bool check = true);
    void LockPages(void* addr, uint64_t count, bool unfree = true, bool check = true);
    void UnlockPage(void* addr);
    void UnlockPages(void* addr, uint64_t count);
    void FreePage(void* addr);
    void FreePages(void* addr, uint64_t count);
    bool UnfreePage(void* addr, bool exact = false);
    bool UnfreePages(void* addr, uint64_t count, bool exact = false);
private:
    uint64_t m_FreePagesCount;
    AVLTree::Node* m_FreePagesSizeTree;
    AVLTree::Node* m_ReservedANDUsedPages; // highest bit in extra data determines if it reserved (1) or used (0)
    uint64_t m_ReservedPagesCount;
    uint64_t m_UsedPagesCount;
    VirtualRegion m_region;
};

extern VirtualPageManager* g_VPM;
extern VirtualPageManager* g_KVPM; // this is probably not the best place for this, but there are no other options

#endif /* _VIRTUAL_PAGE_MGR_HPP */