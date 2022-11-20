#ifndef _X86_64_KERNEL_PAGE_TABLE_MANAGER_H
#define _X86_64_KERNEL_PAGE_TABLE_MANAGER_H

#include <stdint.h>
#include <Memory/Memory.h>

#include "PageFrameAllocator.h"
#include "PageMapIndexer.h"
#include "PageTables.h"
#include "PagingUtil.h"

namespace x86_64_WorldOS {
    class PageTableManager {
    public:
        PageTableManager(WorldOS::MemoryMapEntry** MemoryMap, uint64_t MMEntryCount, PageMapLevel4Entry* PML4);
    };
}

#endif /* _X86_64_KERNEL_PAGE_TABLE_MANAGER_H */