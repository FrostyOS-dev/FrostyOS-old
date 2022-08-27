#include "Memory.h"

void* malloc(size_t size) {

}

void* realloc(void* origin, size_t size) {

}

void  free(void* block) {

}



void memcpy (void* dst, const void* src, size_t size) {

}

void memmove(void* dst, void* src, size_t size) {

}

void memset(void* dst, const uint8_t byte) {

}

uint8_t memcmp(const void* s1, const void* s2, size_t size) {

}



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
