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

namespace WorldOS {

    class PageManager {
    public:
        PageManager();
        PageManager(void* virtual_region_start, size_t virtual_region_size, bool mode = false); // mode is false for supervisor and true for user
        ~PageManager();

        void InitPageManager(void* virtual_region_start, size_t virtual_region_size, bool mode = false); // Extra function for later initialisation. mode is false for supervisor and true for user
        
        void* AllocatePage();
        void* AllocatePages(uint64_t count);

        void FreePage(void* addr);
        void FreePages(void* addr);

    private:
        PageObject* m_allocated_objects;
        uint64_t m_allocated_object_count;
        
        void* m_Vregion_start;
        size_t m_Vregion_size;

        bool m_mode;

        bool m_page_object_pool_used;
    };

    extern PageManager* g_KPM;

}

#endif /* _PAGE_MANAGER_H */