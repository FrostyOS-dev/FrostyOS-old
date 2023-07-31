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

#include "RSDP.hpp"

#include <stddef.h>

bool g_XSDT_supported = false;
bool g_ACPI_revision = false; // false for 1.0, true for 2.0 and higher
RSDPDescriptor* g_baseRSDP;
RSDPDescriptor20* g_fullRSDP;

bool InitAndValidateRSDP(void* RSDP) {
    if (RSDP == nullptr)
        return false;
    g_baseRSDP = (RSDPDescriptor*)RSDP;
    size_t sum = 0;
    for (size_t i = 0; i < sizeof(RSDPDescriptor); i++) {
        sum += (reinterpret_cast<uint8_t*>(g_baseRSDP))[i];
    }
    if ((sum & 0xFF) != 0)
        return false; // invalid checksum
    if (g_baseRSDP->Revision == 0) // ACPI version 1.0
        return true;
    else if (g_baseRSDP->Revision == 2) { // ACPI version 2.0 to 6.1
        g_ACPI_revision = true;
        g_fullRSDP = reinterpret_cast<RSDPDescriptor20*>(g_baseRSDP);
        size_t sum2 = 0;
        for (size_t i = 0; i < (sizeof(RSDPDescriptor20) - sizeof(RSDPDescriptor)); i++) {
            sum += (reinterpret_cast<uint8_t*>(&(g_fullRSDP->Length)))[i];
        }
        if ((sum2 & 0xFF) != 0)
            return false; // extended RSDP table checksum invalid
        if (g_fullRSDP->XSDTAddress != 0)
            g_XSDT_supported = true;
        return true; // Checksum for ACPI version 2.0 to 6.1 is valid
    }
    return true; // invalid ACPI version, so version 1.0 is assumed.
}

bool IsXSDTAvailable() {
    return g_XSDT_supported;
}

void* GetXSDT() {
    if (!IsXSDTAvailable())
        return nullptr;
    return (void*)(g_fullRSDP->XSDTAddress);
}
