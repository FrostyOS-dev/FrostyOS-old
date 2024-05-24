/*
Copyright (©) 2024  Frosty515

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

#ifndef _HAL_ACPI_HPET_HPP
#define _HAL_ACPI_HPET_HPP

#include <stdint.h>

#include <uacpi/tables.h>

struct HPETACPIHeader {
    uint8_t HardwareRevID;
    uint8_t ComparatorCount : 5;
    uint8_t CounterSize : 1;
    uint8_t Reserved : 1;
    uint8_t LegacyReplacement : 1;
    uint16_t PCIVendorID;
    uint8_t AddressSpaceID; // 0x00 = Memory, 0x01 = I/O
    uint8_t RegisterBitWidth;
    uint8_t RegisterBitOffset;
    uint8_t Reserved2;
    uint64_t Address;
    uint8_t HPETNumber;
    uint16_t MinimumTickPeriodic;
    uint8_t PageProtection : 4;
    uint8_t OEMSpecific : 4;
} __attribute__((packed));

bool InitAndValidateHPET(uacpi_table* HPET);

void* GetHPETAddress();

#endif /* _HAL_ACPI_HPET_HPP */