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

#ifndef _X86_64_GDT_HPP
#define _X86_64_GDT_HPP

#include <stdint.h>

#include "TSS.hpp"

struct x86_64_GDTDescriptor {
    uint16_t Size;
    uint64_t Offset;
} __attribute__((packed));

enum class x86_64_GDTAccessByte {
    Accessed = 1,
    Write = 2, // only applicable to data selectors
    Read = 2, // only applicable to code selectors
    GrowDown = 4, // only applicable to data selectors
    EqualOrLower = 4, // only applicable to code selectors
    Executable = 8,
    CodeData = 16,
    Ring0 = 0,
    Ring1 = 32,
    Ring2 = 64,
    Ring3 = 96,
    Present = 128
};

enum class x86_64_GDTSysSegType {
    LDT = 0x2,
    TSS_AVAILABLE = 0x9,
    TSS_BUSY = 0xB
};

enum class x86_64_GDTFlags {
    LongMode = 2,
    PageBlocks = 8
};

struct x86_64_GDTEntry {
    uint16_t Limit0;
    uint16_t Base0;
    uint8_t Base1;
    uint8_t AccessByte;
    uint8_t Limit1 : 4;
    uint8_t Flags : 4;
    uint8_t Base2;
} __attribute__((packed));

struct x86_64_GDTSysEntry {
    uint16_t Limit0;
    uint16_t Base0;
    uint8_t Base1;
    uint8_t AccessByte;
    uint8_t Limit1 : 4;
    uint8_t Flags : 4;
    uint8_t Base2;
    uint32_t Base3;
    uint32_t Reserved;
} __attribute__((packed));

void x86_64_GDT_SetTSS(x86_64_TSS* TSS);

void x86_64_GDTInit();

extern "C" void x86_64_LoadGDT(x86_64_GDTDescriptor* gdtDescriptor);

#endif /* _X86_64_GDT_HPP */