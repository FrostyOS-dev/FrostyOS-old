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

#include "PagingUtil.hpp"
#include "PageMapIndexer.hpp"
#include "PageTables.hpp"

#include <util.h>

void x86_64_InitUserTable(void* PML4) {
    Level4Group* group = (Level4Group*)PML4;

    memset(group, 0, sizeof(Level4Group) >> 3);

    group->entries[511] = K_PML4_Array.entries[511];
    uint16_t HHDM_PML4_offset = ((uint64_t)x86_64_GetHHDMStart() & 0x0000ff8000000000) >> 39;
    group->entries[HHDM_PML4_offset] = K_PML4_Array.entries[HHDM_PML4_offset];
}
