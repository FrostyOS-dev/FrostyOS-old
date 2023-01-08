#ifndef _X86_64_KERNEL_PAGE_TABLE_MANAGER_HPP
#define _X86_64_KERNEL_PAGE_TABLE_MANAGER_HPP

#include <stdint.h>
#include <Memory/Memory.hpp>

namespace x86_64_WorldOS {
    class PageTableManager {
    public:
        PageTableManager(WorldOS::MemoryMapEntry** MemoryMap, uint64_t MMEntryCount, uint64_t kernel_virtual, uint64_t kernel_physical, size_t kernel_size);
        ~PageTableManager();
    
    private:
        size_t m_MemorySize;
    };
}

#endif /* _X86_64_KERNEL_PAGE_TABLE_MANAGER_HPP */