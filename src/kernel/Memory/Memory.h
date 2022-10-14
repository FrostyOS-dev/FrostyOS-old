#ifndef _KERNEL_MEMORY_H
#define _KERNEL_MEMORY_H

#include "stdint.h"
#include "stddef.h"

#ifndef __cplusplus
#error C++ only
#else


namespace WorldOS {

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

    /*class Kernel_Memory {
    public:
        Kernel_Memory();
        ~Kernel_Memory();

        uint64_t heap_alloc(void* dst, size_t size);
        uint64_t heap_free(void* block);

        uint64_t stack_alloc(void* dst, size_t size);
        uint64_t stack_free(void* block);

    private:
        inline MemoryMapEntry GetEntry(size_t index) {
            if (index >= m_MemoryMapSize) {
                return {0, 0, WORLDOS_MEMORY_BAD_MEMORY};
            }
            return m_MemoryMapEntries[index]; 
        };

        void AddEntry(MemoryMapEntry entry);
        void RemoveEntry(MemoryMapEntry entry);

    private:
        size_t m_MemoryMapSize;
        MemoryMapEntry* m_MemoryMapEntries;
    };*/
}
size_t GetMemorySize(const WorldOS::MemoryMapEntry** MemoryMap, const size_t EntryCount);

#endif

#endif /* _KERNEL_MEMORY_H */