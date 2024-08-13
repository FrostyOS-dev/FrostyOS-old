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

#include "MCFG.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <uacpi/acpi.h>
#pragma GCC diagnostic pop

acpi_sdt_hdr* g_MCFG;

bool InitAndValidateMCFG(uacpi_table* MCFG) {
    if (MCFG == nullptr)
        return false;
    g_MCFG = MCFG->hdr;
    return true;
}

MCFGEntry* GetMCFGEntry(uint64_t index) {
    MCFGEntry* start = (MCFGEntry*)((uint64_t)g_MCFG + sizeof(acpi_sdt_hdr) + 8/* Reserved */);
    return (MCFGEntry*)&(start[index]);
}

uint64_t GetMCFGEntryCount() {
    return (g_MCFG->length - sizeof(acpi_sdt_hdr) - 8/* Reserved */) / sizeof(MCFGEntry);
}
