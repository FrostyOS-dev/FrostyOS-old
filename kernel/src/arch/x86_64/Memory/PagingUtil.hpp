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

#ifndef _x86_64_PAGING_UTIL_HPP
#define _x86_64_PAGING_UTIL_HPP

#include <stdint.h>
#include <stddef.h>

// the threshold for when to flush the entire TLB instead of invalidating individual pages
#define FULL_FLUSH_THRESHOLD 0x100000

// Defined in NASM Source file

extern "C" void x86_64_FlushTLB();
extern "C" void x86_64_LoadCR3(uint64_t value);
extern "C" uint64_t x86_64_GetCR3();
extern "C" uint64_t x86_64_SwapCR3(uint64_t value);
extern "C" uint64_t x86_64_GetCR2();
extern "C" void x86_64_InvalidatePage(uint64_t address);

extern "C" bool x86_64_EnsureNX();
extern "C" bool x86_64_EnsureLargePages();

// Defined in C++ Source file

void x86_64_InitUserTable(void* PML4);

void x86_64_InvalidatePages(uint64_t address, uint64_t length);

void x86_64_Prep_SMP_Startup();
void x86_64_Cleanup_SMP_Startup();

bool isInKernelSpace(void* base, size_t length);

#endif /* _x86_64_PAGING_UTIL_HPP */