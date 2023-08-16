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

#ifndef _X86_64_TSS_HPP
#define _X86_64_TSS_HPP

#include <stdint.h>

struct x86_64_TSS {
    uint32_t Reserved0;
    uint64_t RSP[3];
    uint64_t Reserved1;
    uint64_t IST[7];
    uint64_t Reserved2;
    uint16_t Reserved3;
    uint16_t IOPB;
} __attribute__((packed));

void x86_64_TSS_Init();

extern "C" void x86_64_TSS_Load(uint16_t GDTOffset);

#endif /* X86_64_TSS_HPP */