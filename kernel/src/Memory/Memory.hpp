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

#ifndef _KERNEL_MEMORY_HPP
#define _KERNEL_MEMORY_HPP

#include <stdint.h>
#include <stddef.h>

#include "newdelete.hpp"

#ifndef __cplusplus
#error C++ only
#else

#define MEMORY_MAP_ENTRY_SIZE 24

constexpr uint64_t WORLDOS_MEMORY_FREE                   =  0;
constexpr uint64_t WORLDOS_MEMORY_RESERVED               =  1;
constexpr uint64_t WORLDOS_MEMORY_ACPI_RECLAIMABLE       =  2;
constexpr uint64_t WORLDOS_MEMORY_ACPI_NVS               =  3;
constexpr uint64_t WORLDOS_MEMORY_BAD_MEMORY             =  4;
constexpr uint64_t WORLDOS_MEMORY_BOOTLOADER_RECLAIMABLE =  5;
constexpr uint64_t WORLDOS_MEMORY_KERNEL_AND_MODULES     =  6;
constexpr uint64_t WORLDOS_MEMORY_FRAMEBUFFER            =  7;
constexpr uint64_t WORLDOS_MEMORY_KERNEL_DATA            =  8;
constexpr uint64_t WORLDOS_MEMORY_APPLICATIONS           =  9;
constexpr uint64_t WORLDOS_MEMORY_APPLICATION_DATA       = 10;
constexpr uint64_t WORLDOS_MEMORY_USER_OTHER             = 11;

struct MemoryMapEntry {
    uint64_t Address;
    uint64_t length;
    uint64_t type;
} __attribute__((packed));

size_t GetMemorySize(const MemoryMapEntry** MemoryMap, const size_t EntryCount);
size_t UpdateMemorySize(const MemoryMapEntry** MemoryMap, const size_t EntryCount);

#endif

#endif /* _KERNEL_MEMORY_HPP */