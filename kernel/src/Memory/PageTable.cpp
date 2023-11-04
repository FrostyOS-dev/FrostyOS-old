/*
Copyright (©) 2023  Frosty515

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

#include "PageTable.hpp"
#include "PageManager.hpp"
#include "PhysicalPageFrameAllocator.hpp"

#include <util.h>

PageTable g_KPT {false, g_KPM};

#ifdef __x86_64__
#include <arch/x86_64/Memory/PageMapIndexer.hpp>


PageTable::PageTable(bool mode, PageManager* pm) : m_root_table(nullptr), m_mode(mode), m_pm(pm) {
    if (m_mode && m_pm != nullptr) {
        m_root_table = x86_64_to_HHDM(g_PPFA->AllocatePage());
        x86_64_InitUserTable(m_root_table);
    }
    else
        m_root_table = &K_PML4_Array;
}

PageTable::~PageTable() {
    if (m_mode && m_pm != nullptr)
        m_pm->FreePage(m_root_table);
}

void PageTable::MapPage(void* physical_addr, void* virtual_addr, PagePermissions perms, bool flush) {
    if (flush)
        x86_64_map_page((Level4Group*)m_root_table, physical_addr, virtual_addr, DecodePageFlags(perms));
    else
        x86_64_map_page_noflush((Level4Group*)m_root_table, physical_addr, virtual_addr, DecodePageFlags(perms));
}

void PageTable::RemapPage(void* virtual_addr, PagePermissions perms, bool flush) {
    if (flush)
        x86_64_remap_page((Level4Group*)m_root_table, virtual_addr, DecodePageFlags(perms));
    else
        x86_64_remap_page_noflush((Level4Group*)m_root_table, virtual_addr, DecodePageFlags(perms));
}

void PageTable::UnmapPage(void* virtual_addr, bool flush) {
    if (flush)
        x86_64_unmap_page((Level4Group*)m_root_table, virtual_addr);
    else
        x86_64_unmap_page_noflush((Level4Group*)m_root_table, virtual_addr);
}

void* PageTable::GetPhysicalAddress(void* virtual_addr) const {
    return x86_64_get_physaddr((Level4Group*)m_root_table, virtual_addr);
}

uint32_t PageTable::DecodePageFlags(PagePermissions perms) const {
    uint32_t page_perms = 1;
    if (m_mode)
        page_perms |= 4;
    switch (perms) {
    case PagePermissions::READ:
        page_perms |= 0x8000000; // No execute
        break; // not possible to set read flag
    case PagePermissions::WRITE:
    case PagePermissions::READ_WRITE:
        page_perms |= 0x8000002; // Write, No execute
        break;
    case PagePermissions::EXECUTE:
    case PagePermissions::READ_EXECUTE:
        break;
    default:
        page_perms = 0;
        break;
    }
    return page_perms;
}

#endif /* __x86_64__*/

void* PageTable::GetRootTable() const {
    return m_root_table;
}

void* PageTable::GetRootTablePhysical() const {
    return GetPhysicalAddress(m_root_table);
}
