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

volatile uint64_t g_memorySizeBytes = 0;

size_t GetMemorySize(const MemoryMapEntry** MemoryMap, const size_t EntryCount) {

    if (g_memorySizeBytes > 0) {
        return g_memorySizeBytes;
    }

    for (size_t i = 0; i < EntryCount; i++) {
        const MemoryMapEntry* entry = MemoryMap[i];
        //if (entry->Address >= g_memorySizeBytes) continue; // prevents invalid entries from being counted
        g_memorySizeBytes += entry->length;
    }

    return g_memorySizeBytes;
}
