#ifndef _KERNEL_X86_64_PAGING_INIT_HPP
#define _KERNEL_X86_64_PAGING_INIT_HPP

#include <Memory/Memory.hpp>

#include <stdint.h>

void x86_64_InitPaging(WorldOS::MemoryMapEntry** MemoryMap, uint64_t MMEntryCount, uint64_t kernel_virtual, uint64_t kernel_physical, size_t kernel_size, uint64_t fb_virt, uint64_t fb_size, uint64_t HHDM_start);

#endif /* _KERNEL_X86_64_PAGING_INIT_HPP */