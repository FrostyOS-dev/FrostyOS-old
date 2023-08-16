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

#ifndef _X86_64_CPUID_HPP
#define _X86_64_CPUID_HPP

#include <stdint.h>

struct x86_64_cpuid_regs {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
} __attribute__((packed));

// Defined in cpuid.asm

// Call cpuid with given registers
extern "C" x86_64_cpuid_regs x86_64_cpuid(x86_64_cpuid_regs regs);

#endif /* _X86_64_CPUID_HPP */