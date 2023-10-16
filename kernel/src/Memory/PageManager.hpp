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

#ifndef _PAGE_MANAGER_H
#define _PAGE_MANAGER_H

#include <stdint.h>
#include <stddef.h>

#include "PageObject.hpp"
#include "Memory.hpp"
#include "VirtualPageManager.hpp"

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

    /* mode is false for supervisor and true for user. auto_expand allows the page manager to try and expand the virtual region for user page managers. */
    PageManager(const VirtualRegion& region, VirtualPageManager* VPM, bool mode, bool auto_expand = false);
    ~PageManager();

    void InitPageManager(const VirtualRegion& region, VirtualPageManager* VPM, bool mode, bool auto_expand = false); // Extra function for later initialisation. mode is false for supervisor and true for user
    
    void* AllocatePage(PagePermissions perms = PagePermissions::READ_WRITE, void* addr = nullptr);
    void* AllocatePages(uint64_t count, PagePermissions perms = PagePermissions::READ_WRITE, void* addr = nullptr);

    /* Allocate virtual memory, but don't map it yet*/
    void* ReservePage(PagePermissions perms = PagePermissions::READ_WRITE, void* addr = nullptr);
    void* ReservePages(uint64_t count, PagePermissions perms = PagePermissions::READ_WRITE, void* addr = nullptr);

    void FreePage(void* addr);
    void FreePages(void* addr);

    void Remap(void* addr, PagePermissions perms);

    bool ExpandVRegionToRight(size_t new_size);

    bool isWritable(void* addr, size_t size) const;

    bool isValidAllocation(void* addr, size_t size) const;

    const VirtualRegion& GetRegion() const;

private:
    bool InsertObject(PageObject* obj);

private:
    PageObject* m_allocated_objects;
    uint64_t m_allocated_object_count;
    
    VirtualRegion m_Vregion;
    VirtualPageManager* m_VPM; // uses a pointer to avoid wasted RAM

    bool m_mode;

    bool m_page_object_pool_used;

    bool m_auto_expand;
};

extern PageManager* g_KPM;

#endif /* _PAGE_MANAGER_H */