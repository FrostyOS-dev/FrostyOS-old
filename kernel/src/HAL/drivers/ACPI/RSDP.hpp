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

#ifndef _HAL_ACPI_RSDP_HPP
#define _HAL_ACPI_RSDP_HPP

#include <stdint.h>

struct RSDPDescriptor {
 char Signature[8];
 uint8_t Checksum;
 char OEMID[6];
 uint8_t Revision;
 uint32_t RSDTAddress;
} __attribute__ ((packed));

struct RSDPDescriptor20 {
 RSDPDescriptor firstPart;
 
 uint32_t Length;
 uint64_t XSDTAddress;
 uint8_t ExtendedChecksum;
 uint8_t reserved[3];
} __attribute__ ((packed));

bool InitAndValidateRSDP(void* RSDP);

bool IsXSDTAvailable();

void* GetXSDT();

#endif /* _HAL_ACPI_RSDP_HPP */