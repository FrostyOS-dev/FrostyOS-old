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

#ifndef _HAL_ACPI_MADT_HPP
#define _HAL_ACPI_MADT_HPP

#include <stdint.h>

#include <uacpi/tables.h>

struct MADTEntriesHeader {
    uint32_t LAPICAddress;
    uint32_t Flags;
} __attribute__((packed));

struct MADTEntryHeader {
    uint8_t Type;
    uint8_t Length;
} __attribute__((packed));

struct MADT_LocalAPIC {
    uint8_t Type; // 0
    uint8_t Length;
    uint8_t ACPIProcessorID;
    uint8_t APICID;
    uint32_t Flags;
} __attribute__((packed));

struct MADT_IOAPIC {
    uint8_t Type; // 1
    uint8_t Length;
    uint8_t IOAPICID;
    uint8_t Reserved;
    uint32_t IOAPICAddress;
    uint32_t GlobalSystemInterruptBase;
} __attribute__((packed));

struct MADT_InterruptSourceOverride {
    uint8_t Type; // 2
    uint8_t Length;
    uint8_t Bus;
    uint8_t Source;
    uint32_t GlobalSystemInterrupt;
    uint16_t Flags;
} __attribute__((packed));

struct MADT_IOAPICNMISource {
    uint8_t Type; // 3
    uint8_t Length;
    uint16_t Flags;
    uint32_t GlobalSystemInterrupt;
} __attribute__((packed));

struct MADT_LocalAPICNMISource {
    uint8_t Type; // 4
    uint8_t Length;
    uint8_t ACPIProcessorID;
    uint16_t Flags;
    uint8_t LocalAPICLint;
} __attribute__((packed));

struct MADT_LocalAPICAddressOverride {
    uint8_t Type; // 5
    uint8_t Length;
    uint16_t Reserved;
    uint64_t LocalAPICAddress;
} __attribute__((packed));

struct MADT_Localx2APIC {
    uint8_t Type; // 9
    uint8_t Length;
    uint8_t Reserved[2];
    uint32_t x2APICID;
    uint32_t Flags;
    uint32_t ACPIProcessorUID;
} __attribute__((packed));

bool InitAndValidateMADT(uacpi_table* MADT);

void EnumerateMADTEntries();

#endif /* _HAL_ACPI_MADT_HPP */