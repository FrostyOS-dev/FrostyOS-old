#include "Memory.hpp"

volatile uint64_t g_memorySizeBytes = 0;

size_t GetMemorySize(const WorldOS::MemoryMapEntry** MemoryMap, const size_t EntryCount) {
    using namespace WorldOS;

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
