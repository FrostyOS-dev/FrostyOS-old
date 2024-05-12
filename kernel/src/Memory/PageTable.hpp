/*
Copyright (©) 2023-2024  Frosty515

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

#ifndef _PAGE_TABLE_HPP
#define _PAGE_TABLE_HPP

#include <stdint.h>

enum class PagePermissions;
class PageManager;

class PageTable {
public:
    PageTable(bool mode, PageManager* pm); // mode is false for supervisor, true for user
    ~PageTable();

    void MapPage(void* physical_addr, void* virtual_addr, PagePermissions perms, bool flush = true);
    void RemapPage(void* virtual_addr, PagePermissions perms, bool flush = true);
    void UnmapPage(void* virtual_addr, bool flush = true);

    void* GetPhysicalAddress(void* virtual_addr) const;

    void Flush(void* addr, uint64_t length, bool wait = false);

    void* GetRootTable() const;
    void* GetRootTablePhysical() const;

private:

    uint32_t DecodePageFlags(PagePermissions perms) const;

private:
    void* m_root_table;
    bool m_mode;
    PageManager* m_pm;
};

extern PageTable g_KPT;

#endif /* _PAGE_TABLE_HPP */