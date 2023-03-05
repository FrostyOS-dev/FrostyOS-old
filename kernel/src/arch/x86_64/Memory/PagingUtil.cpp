#include "PagingUtil.hpp"

#include "PageTables.hpp"

#include <Memory/PhysicalPageFrameAllocator.hpp>

#include "PageMapIndexer.hpp"

void x86_64_GeneratePageLevel2Array(uint16_t PageLevel3Offset) {
    Level2Group* group = (Level2Group*)g_PPFA->AllocatePage();
    PML3_Array.entries[PageLevel3Offset].Address = (uint64_t)group;
}

void x86_64_GeneratePageLevel1Array(uint16_t PageLevel3Offset, uint16_t PageLevel2Offset) {
    Level1Group* group = (Level1Group*)g_PPFA->AllocatePage();
    Level2Group* group2 = (Level2Group*)PML3_Array.entries[PageLevel3Offset].Address;

    group2->entries[PageLevel2Offset].Address = (uint64_t)group;
}
