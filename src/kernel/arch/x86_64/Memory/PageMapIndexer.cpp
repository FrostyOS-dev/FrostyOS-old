#include "PageMapIndexer.hpp"

Level3Group PML3_Array;

void* x86_64_get_physaddr(void* virtualaddr) {
    uint64_t virtualAddress = (uint64_t)virtualaddr;
    uint16_t offset = virtualAddress & 0xFFF;
    virtualAddress >>= 12;
    uint16_t P_i = virtualAddress & 0x1ff;
    virtualAddress >>= 9;
    uint16_t PT_i = virtualAddress & 0x1ff;
    virtualAddress >>= 9;
    uint16_t PD_i = virtualAddress & 0x1ff;
    virtualAddress >>= 9;
    uint16_t PDP_i = virtualAddress & 0x1ff;

    PageMapLevel3Entry PML3 = PML3_Array.entries[PDP_i];
    if (PML3.Present == 0)
        return nullptr;

    PageMapLevel2Entry PML2 = ((PageMapLevel2Entry*)((uint64_t)PML3.Address << 12))[PD_i];
    if (PML2.Present == 0)
        return nullptr;

    PageMapLevel1Entry PML1 = ((PageMapLevel1Entry*)((uint64_t)PML2.Address << 12))[PT_i];
    if (PML1.Present == 0)
        return nullptr;

    void* Page_addr = ((void**)((uint64_t)PML1.Address << 12))[P_i];
    
    return (void*)((uint64_t)Page_addr | offset);
}

void x86_64_map_page(void* physaddr, void* virtualaddr, uint32_t flags) {
    uint64_t physical_addr = (uint64_t)physaddr & ~0xFFF;
    uint64_t virtual_addr = (uint64_t)virtualaddr & ~0xFFF;

    uint16_t Page  = (uint16_t)(((uint64_t)virtualaddr & 0x1FF000) >> 12);
    uint16_t pt    = (uint16_t)(((uint64_t)virtualaddr & 0x3FE00000) >> 21);
    uint16_t pd = (uint16_t)(((uint64_t)virtualaddr & 0x7FC0000000) >> 30);
    uint16_t pdptr  = (uint16_t)(((uint64_t)virtualaddr & 0xFF8000000000) >> 39);

    PageMapLevel3Entry PML3 = PML3_Array.entries[pdptr];
    if (PML3.Present == 0) {
        PML3.Present = 1;
        x86_64_GeneratePageLevel2Array(pdptr);
    }

    PageMapLevel2Entry PML2 = ((PageMapLevel2Entry*)((uint64_t)PML3.Address << 12))[pd];
    if (PML2.Present == 0) {
        PML3.Present = 1;
        x86_64_GeneratePageLevel1Array(pdptr, pd);
    }

    PageMapLevel1Entry PML1 = ((PageMapLevel1Entry*)((uint64_t)PML2.Address << 12))[pt];
    if (PML1.Present == 0) {
        PML1.Present = 1;
    }

    PML1 = (PageMapLevel1Entry)((uint64_t)(physical_addr | (flags & 0x0FFF) | ((uint64_t)(flags & 0x0FFF0000) << 40)));
    
    x86_64_FlushTLB();
}