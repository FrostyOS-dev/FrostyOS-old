#include "PagingUtil.h"

#include "PageTables.h"

#include "PageFrameAllocator.h"

#include "PageMapIndexer.h"

void x86_64_GeneratePageLevel2Array(uint16_t PageLevel3Offset) {
    Level2Group* group = (Level2Group*)g_PFA->AllocatePage();
    PML3_Array.entries[PageLevel3Offset].Address = (uint64_t)group;
}

void x86_64_GeneratePageLevel1Array(uint16_t PageLevel3Offset, uint16_t PageLevel2Offset) {
    Level1Group* group = (Level1Group*)g_PFA->AllocatePage();
    Level2Group* group2 = (Level2Group*)PML3_Array.entries[PageLevel3Offset].Address;

    group2->entries[PageLevel2Offset].Address = (uint64_t)group;
}
