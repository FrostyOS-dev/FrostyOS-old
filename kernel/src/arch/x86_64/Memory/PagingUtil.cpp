#include "PagingUtil.hpp"

#include "PageTables.hpp"

#include <Memory/PhysicalPageFrameAllocator.hpp>

#include "PageMapIndexer.hpp"

void x86_64_GeneratePageLevel3Array(uint16_t PageLevel4Offset) {
    Level2Group* group = (Level2Group*)g_PPFA->AllocatePage();
    PML4_Array.entries[PageLevel4Offset].Address = (uint64_t)group;
}

void x86_64_GeneratePageLevel2Array(uint16_t PageLevel4Offset, uint16_t PageLevel3Offset) {
    Level2Group* group = (Level2Group*)g_PPFA->AllocatePage();
    Level3Group* group2 = (Level3Group*)(PML4_Array.entries[PageLevel4Offset].Address);

    group2->entries[PageLevel3Offset].Address = (uint64_t)group;
}

void x86_64_GeneratePageLevel1Array(uint16_t PageLevel4Offset, uint16_t PageLevel3Offset, uint16_t PageLevel2Offset) {
    Level1Group* group = (Level1Group*)g_PPFA->AllocatePage();
    Level3Group* group2 = (Level3Group*)(PML4_Array.entries[PageLevel4Offset].Address);
    Level2Group* group3 = (Level2Group*)(group2->entries[PageLevel3Offset].Address);


    group3->entries[PageLevel2Offset].Address = (uint64_t)group;
}
