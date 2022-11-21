#ifndef _X86_64_KERNEL_PAGE_TABLE_MANAGER_HPP
#define _X86_64_KERNEL_PAGE_TABLE_MANAGER_HPP

#include <stdint.h>
#include <Memory/Memory.hpp>

#include "PageFrameAllocator.hpp"
#include "PageMapIndexer.hpp"
#include "PageTables.hpp"
#include "PagingUtil.hpp"

namespace x86_64_WorldOS {
    class PageTableManager {
    public:
        PageTableManager(WorldOS::MemoryMapEntry** MemoryMap, uint64_t MMEntryCount, PageMapLevel4Entry* PML4);
    };
}

#endif /* _X86_64_KERNEL_PAGE_TABLE_MANAGER_HPP */