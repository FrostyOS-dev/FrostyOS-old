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

#include "PagingUtil.hpp"

#include <arch/x86_64/Memory/PageMapIndexer.hpp>

void MapPage(void* phys_addr, void* virt_addr, uint32_t flags) {
    x86_64_map_page(phys_addr, virt_addr, flags);
}

void UnmapPage(void* virt_addr) {
    x86_64_unmap_page(virt_addr);
}

void RemapPage(void* virt_addr, uint32_t flags) {
    x86_64_remap_page(virt_addr, flags);
}

void* to_HHDM(void* phys_addr) {
    return x86_64_to_HHDM(phys_addr);
}

void* get_physaddr(void* virtaddr) {
    return x86_64_get_physaddr(virtaddr);
}
