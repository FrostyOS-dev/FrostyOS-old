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

#include "Memory.hpp"

#include <stdio.h>

volatile uint64_t g_memorySizeBytes = 0;

size_t GetMemorySize(const MemoryMapEntry** MemoryMap, const size_t EntryCount) {

    if (g_memorySizeBytes > 0)
        return g_memorySizeBytes;

    size_t endLastFree = 0;

    for (size_t i = 0; i < EntryCount; i++) {
        const MemoryMapEntry* entry = MemoryMap[i];
        if (entry->type == FROSTYOS_MEMORY_FREE) {
            if (((uint64_t)(entry->Address) + entry->length) > endLastFree)
                endLastFree = (uint64_t)(entry->Address) + entry->length;
        }
    }
    g_memorySizeBytes = endLastFree;

    return g_memorySizeBytes;
}

size_t UpdateMemorySize(const MemoryMapEntry** MemoryMap, const size_t EntryCount) {

    if (g_memorySizeBytes > 0) {
        return EntryCount;
    }

    size_t NewEntryCount = 0;

    for (size_t i = EntryCount; i > 0; i--) {
        const MemoryMapEntry* entry = MemoryMap[i-1];
        if (entry->type == FROSTYOS_MEMORY_FREE) {
            g_memorySizeBytes = (uint64_t)(entry->Address) + entry->length;
            NewEntryCount = i;
            break;
        }
    }

    return NewEntryCount;
}
