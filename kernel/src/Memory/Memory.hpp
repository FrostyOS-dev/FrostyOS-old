#ifndef _KERNEL_MEMORY_HPP
#define _KERNEL_MEMORY_HPP

#include <stdint.h>
#include <stddef.h>

#include "newdelete.hpp"

#ifndef __cplusplus
#error C++ only
#else

#define MEMORY_MAP_ENTRY_SIZE 24

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
}
size_t GetMemorySize(const WorldOS::MemoryMapEntry** MemoryMap, const size_t EntryCount);

#endif

#endif /* _KERNEL_MEMORY_HPP */