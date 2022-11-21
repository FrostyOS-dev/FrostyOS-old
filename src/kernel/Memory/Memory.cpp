#include "Memory.hpp"

size_t GetMemorySize(const WorldOS::MemoryMapEntry** MemoryMap, const size_t EntryCount) {
    using namespace WorldOS;

    static size_t memorySizeBytes = 0;
    if (memorySizeBytes > 0) {
        return memorySizeBytes;
    }

    for (size_t i = 0; i < EntryCount; i++) {
        const MemoryMapEntry* entry = MemoryMap[i];
        memorySizeBytes += entry->length;
    }

    return memorySizeBytes;
}
