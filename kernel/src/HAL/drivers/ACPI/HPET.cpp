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

#include "HPET.hpp"

#include <Memory/PagingUtil.hpp>

ACPISDTHeader* g_HPETTable;

bool InitAndValidateHPET(ACPISDTHeader* HPET) {
    if (HPET == nullptr)
        return false;
    if (doChecksum(HPET)) {
        g_HPETTable = HPET;
        return true;
    }
    return false;
}

void* GetHPETAddress() {
    HPETACPIHeader* hpetHeader = (HPETACPIHeader*)((uint64_t)g_HPETTable + sizeof(ACPISDTHeader));
    return to_HHDM((void*)hpetHeader->Address);
}
