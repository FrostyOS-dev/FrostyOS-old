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

#include "XSDT.hpp"

#include <Memory/PagingUtil.hpp>

ACPISDTHeader* g_XSDT;

bool InitAndValidateXSDT(void* XSDT) {
    if (XSDT == nullptr)
        return false;
    if (doChecksum(reinterpret_cast<ACPISDTHeader*>(XSDT))) {
        g_XSDT = reinterpret_cast<ACPISDTHeader*>(XSDT);
        return true;
    }
    return false;
}

ACPISDTHeader* getOtherSDT(uint64_t index) {
    uint64_t* start = (uint64_t*)((uint64_t)g_XSDT + sizeof(ACPISDTHeader));
    return (ACPISDTHeader*)to_HHDM((void*)(start[index]));
}

uint64_t getSDTCount() {
    return (g_XSDT->Length - sizeof(ACPISDTHeader)) / sizeof(uint64_t);
}
