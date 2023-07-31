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

#include "GDT.hpp"

#include <util.h>

x86_64_GDTEntry __attribute__((aligned(0x1000))) g_GDT[7];

void x86_64_GDT_SetTSS(x86_64_TSS* TSS) {
    uint64_t TSS_addr = (uint64_t)TSS;
    x86_64_GDTSysEntry entry;
    fast_memset((void*)&entry, 0, sizeof(x86_64_GDTSysEntry) / 8);
    entry.AccessByte = ((uint8_t)x86_64_GDTAccessByte::Present | (uint8_t)x86_64_GDTSysSegType::TSS_AVAILABLE);
    entry.Limit0 = sizeof(x86_64_TSS) & 0xFFFF;
    entry.Limit1 = (sizeof(x86_64_TSS) >> 16) & 0xF;
    entry.Base0 = TSS_addr & 0xFFFF;
    entry.Base1 = (TSS_addr >> 16) & 0xFF;
    entry.Base2 = (TSS_addr >> 24) & 0xFF;
    entry.Base3 = (TSS_addr >> 32) & 0xFFFFFFFF;
    x86_64_GDTEntry* entry0 = reinterpret_cast<x86_64_GDTEntry*>(&entry);
    x86_64_GDTEntry* entry1 = reinterpret_cast<x86_64_GDTEntry*>(&(entry.Base3));
    g_GDT[5] = *entry0;
    g_GDT[6] = *entry1;
}

void x86_64_GDTInit() {
    fast_memset(&(g_GDT[0]), 0, sizeof(x86_64_GDTEntry) * 7 / 8);
    g_GDT[1].AccessByte = ((uint8_t)x86_64_GDTAccessByte::Read | (uint8_t)x86_64_GDTAccessByte::Executable | (uint8_t)x86_64_GDTAccessByte::CodeData | (uint8_t)x86_64_GDTAccessByte::Ring0 | (uint8_t)x86_64_GDTAccessByte::Present);
    g_GDT[1].Flags = (uint8_t)x86_64_GDTFlags::LongMode | (uint8_t)x86_64_GDTFlags::PageBlocks;
    g_GDT[2].AccessByte = ((uint8_t)x86_64_GDTAccessByte::Write | (uint8_t)x86_64_GDTAccessByte::CodeData | (uint8_t)x86_64_GDTAccessByte::Ring0 | (uint8_t)x86_64_GDTAccessByte::Present);
    g_GDT[2].Flags = (uint8_t)x86_64_GDTFlags::LongMode | (uint8_t)x86_64_GDTFlags::PageBlocks;

    g_GDT[3].AccessByte = ((uint8_t)x86_64_GDTAccessByte::Read | (uint8_t)x86_64_GDTAccessByte::Executable | (uint8_t)x86_64_GDTAccessByte::CodeData | (uint8_t)x86_64_GDTAccessByte::Ring3 | (uint8_t)x86_64_GDTAccessByte::Present);
    g_GDT[3].Flags = (uint8_t)x86_64_GDTFlags::LongMode | (uint8_t)x86_64_GDTFlags::PageBlocks;
    g_GDT[4].AccessByte = ((uint8_t)x86_64_GDTAccessByte::Write | (uint8_t)x86_64_GDTAccessByte::CodeData | (uint8_t)x86_64_GDTAccessByte::Ring3 | (uint8_t)x86_64_GDTAccessByte::Present);
    g_GDT[4].Flags = (uint8_t)x86_64_GDTFlags::LongMode | (uint8_t)x86_64_GDTFlags::PageBlocks;

    x86_64_TSS_Init();

    x86_64_GDTDescriptor GDTDescriptor;
    GDTDescriptor.Size = sizeof(x86_64_GDTEntry) * 7 - 1;
    GDTDescriptor.Offset = (uint64_t)&(g_GDT[0]);
    x86_64_LoadGDT(&GDTDescriptor);
    x86_64_TSS_Load(0x28);
}
