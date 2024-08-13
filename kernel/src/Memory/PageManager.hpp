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

#ifndef _PAGE_MANAGER_H
#define _PAGE_MANAGER_H

#include <stdint.h>
#include <stddef.h>
#include <spinlock.h>
#include <util.h>
#include <stdio.h>

#include <file.h>

#include "PageObject.hpp"
#include "VirtualRegion.hpp"
#ifdef _FROSTYOS_BUILD_TARGET_IS_KERNEL
#include "VirtualPageManager.hpp"
#include "PageTable.hpp"
#endif

#include <Data-structures/AVLTree.hpp>

enum class PagePermissions {
    READ,
    WRITE,
    EXECUTE,
    READ_WRITE,
    READ_EXECUTE
};

class PageManager {
public:
    PageManager();

#ifdef _FROSTYOS_BUILD_TARGET_IS_KERNEL
    /* mode is false for supervisor and true for user. auto_expand allows the page manager to try and expand the virtual region for user page managers. */
    PageManager(const VirtualRegion& region, VirtualPageManager* VPM, bool mode, bool auto_expand = false);
    void InitPageManager(const VirtualRegion& region, VirtualPageManager* VPM, bool mode, bool auto_expand = false); // Extra function for later initialisation. mode is false for supervisor and true for user
#elif defined(_FROSTYOS_BUILD_TARGET_IS_USERLAND)
    void InitPageManager();
#endif
    ~PageManager();
    
    void* AllocatePage(PagePermissions perms = PagePermissions::READ_WRITE, void* addr = nullptr);
    void* AllocatePages(uint64_t count, PagePermissions perms = PagePermissions::READ_WRITE, void* addr = nullptr);

    /* Allocate virtual memory, but don't map it yet*/
    void* ReservePage(PagePermissions perms = PagePermissions::READ_WRITE, void* addr = nullptr);
    void* ReservePages(uint64_t count, PagePermissions perms = PagePermissions::READ_WRITE, void* addr = nullptr);

    void FreePage(void* addr);
    void FreePages(void* addr);

    void Remap(void* addr, PagePermissions perms);

    void* MapPage(void* phys, PagePermissions perms, void* addr = nullptr);
    void* MapPages(void* phys, uint64_t count, PagePermissions perms, void* addr = nullptr);

    void UnmapPage(void* addr);
    void UnmapPages(void* addr);

#ifdef _FROSTYOS_BUILD_TARGET_IS_KERNEL
    bool ExpandVRegionToRight(size_t new_size);
#endif

    bool isReadable(void* addr, size_t size) const;
    bool isWritable(void* addr, size_t size) const;

    bool isValidAllocation(void* addr, size_t size) const;

    // Get the permissions for a single page
    PagePermissions GetPermissions(void* addr) const;

#ifdef _FROSTYOS_BUILD_TARGET_IS_KERNEL
    const VirtualRegion& GetRegion() const;

    const PageTable& GetPageTable() const;
#endif

    void PrintRegions(fd_t fd) const;

private:
    PageObject* CreateObject();
    PageObject* CreateObject(const PageObject& obj);
    void DeleteObject(const PageObject* obj);
    bool InsertObject(PageObject* obj);
    bool RemoveObject(PageObject* obj);
    void EnumerateObjects(bool (*callback)(PageObject* obj, void* data), void* data) const;
    PageObject* FindObject(void* addr) const;
    PageObject* FindObject(const VirtualRegion& region) const;
    PageObject* FindObjectImpl(AVLTree::Node const* root, void* base, uint64_t size) const;

    bool isReadableImpl(void* addr, size_t size) const;
    bool isWritableImpl(void* addr, size_t size) const;
    

private:
    AVLTree::SimpleAVLTree<void*, PageObject*> m_allocated_objects;
    uint64_t m_allocated_object_count;
    
#ifdef _FROSTYOS_BUILD_TARGET_IS_KERNEL
    VirtualRegion m_Vregion;
    VirtualPageManager* m_VPM; // uses a pointer to avoid wasted RAM
    PageTable m_PT;

    bool m_mode;
    bool m_auto_expand;
#endif
    bool m_page_object_pool_used;

    mutable spinlock_t m_lock;
};

#ifdef _FROSTYOS_BUILD_TARGET_IS_USERLAND
void* __user_raw_mmap(void* addr, size_t size, PagePermissions perms);
void __user_raw_mprotect(void* addr, size_t size, PagePermissions perms);
#endif

extern PageManager* g_KPM;

#endif /* _PAGE_MANAGER_H */