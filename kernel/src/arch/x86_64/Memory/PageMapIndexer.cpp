#include "PageMapIndexer.hpp"

Level4Group __attribute__((aligned(0x1000))) PML4_Array;
Level3Group __attribute__((aligned(0x1000))) PML3_LowestArray;
Level2Group __attribute__((aligned(0x1000))) PML2_LowestArray;
Level1Group __attribute__((aligned(0x1000))) PML1_LowestArray;
Level3Group __attribute__((aligned(0x1000))) PML3_KernelGroup; // only highest 2 entries are used
Level2Group __attribute__((aligned(0x1000))) PML2_KernelLower;
Level1Group __attribute__((aligned(0x1000))) PML1_KernelLowest;

void* g_kernel_physical = nullptr;
void* g_kernel_virtual  = nullptr;
size_t g_kernel_length = 0;

#include <stdio.hpp>
#include <util.h>

#include <Memory/PhysicalPageFrameAllocator.hpp>

void* x86_64_get_physaddr(void* virtualaddr) {

    // check if there is a simpler way
    if ((uint64_t)virtualaddr <= (uint64_t)virtualaddr && ((uint64_t)virtualaddr + g_kernel_length) > (uint64_t)virtualaddr) {
        return (void*)((uint64_t)virtualaddr - (uint64_t)g_kernel_virtual + (uint64_t)g_kernel_physical);
    }

    uint64_t virtualAddress = (uint64_t)virtualaddr;
    uint16_t offset = virtualAddress & 0xFFF;
    virtualAddress >>= 12;
    uint16_t PT_i = virtualAddress & 0x1ff;
    virtualAddress >>= 9;
    uint16_t PD_i = virtualAddress & 0x1ff;
    virtualAddress >>= 9;
    uint16_t PDP_i = virtualAddress & 0x1ff;
    virtualAddress >>= 9;
    uint16_t PML4_i = virtualAddress & 0x1ff;

    PageMapLevel4Entry PML4 = PML4_Array.entries[PML4_i];
    if (PML4.Present == 0)
        return nullptr;

    PageMapLevel3Entry PML3 = ((PageMapLevel3Entry*)((uint64_t)PML4.Address << 12))[PDP_i];
    if (PML3.Present == 0)
        return nullptr;

    PageMapLevel2Entry PML2 = ((PageMapLevel2Entry*)((uint64_t)PML3.Address << 12))[PD_i];
    if (PML2.Present == 0)
        return nullptr;

    PageMapLevel1Entry PML1 = ((PageMapLevel1Entry*)((uint64_t)PML2.Address << 12))[PT_i];
    if (PML1.Present == 0)
        return nullptr;

    void* Page_addr = (void*)((uint64_t)PML1.Address << 12);
    
    return (void*)((uint64_t)Page_addr | offset);
}

void x86_64_map_page_noflush(void* physaddr, void* virtualaddr, uint32_t flags) {
    uint64_t physical_addr = (uint64_t)physaddr & ~0xFFF;
    uint64_t virtual_addr = (uint64_t)virtualaddr & ~0xFFF;

    const uint16_t pt    = (uint16_t)((virtual_addr & 0x0000001FF000) >> 12);
    const uint16_t pd    = (uint16_t)((virtual_addr & 0x00003FE00000) >> 21);
    const uint16_t pdptr = (uint16_t)((virtual_addr & 0x007FC0000000) >> 30);
    const uint16_t pml4  = (uint16_t)((virtual_addr & 0xFF8000000000) >> 39);

    PageMapLevel4Entry PML4 = PML4_Array.entries[pml4];
    if (PML4.Present == 0) {
        uint64_t temp = ((uint64_t)((flags & 0x0FFF) | ((uint64_t)(flags & 0x0FFF0000) << 40)));
        PML4 = *(PageMapLevel4Entry*)(&temp);
        PML4.Present = 1;
        PML4.Address = (uint64_t)g_PPFA->AllocatePage() >> 12;
        fast_memset((void*)(PML4.Address << 12), 0, 512);
        PML4_Array.entries[pml4] = PML4;
    }
    else {
        uint64_t temp = *(uint64_t*)(&PML4);
        temp |= flags & 0xFFF;
        temp |= (uint64_t)(flags & 0xFFF0000) << 40;
        PML4_Array.entries[pml4] = *(PageMapLevel4Entry*)&temp;
    }

    PageMapLevel3Entry PML3 = ((PageMapLevel3Entry*)((uint64_t)(PML4.Address) << 12))[pdptr];
    if (PML3.Present == 0) {
        uint64_t temp = ((uint64_t)((flags & 0x0FFF) | ((uint64_t)(flags & 0x0FFF0000) << 40)));
        PML3 = *(PageMapLevel3Entry*)(&temp);
        PML3.Present = 1;
        PML3.Address = (uint64_t)g_PPFA->AllocatePage() >> 12;
        fast_memset((void*)(PML3.Address << 12), 0, 512);
        ((PageMapLevel3Entry*)((uint64_t)(PML4.Address) << 12))[pdptr] = PML3;
    }
    else {
        uint64_t temp = *(uint64_t*)(&PML3);
        temp |= flags & 0xFFF;
        temp |= (uint64_t)(flags & 0xFFF0000) << 40;
        ((PageMapLevel3Entry*)((uint64_t)(PML4.Address) << 12))[pdptr] = *(PageMapLevel3Entry*)&temp;
    }

    PageMapLevel2Entry PML2 = ((PageMapLevel2Entry*)((uint64_t)(PML3.Address) << 12))[pd];
    if (PML2.Present == 0) {
        uint64_t temp = ((uint64_t)((flags & 0x0FFF) | ((uint64_t)(flags & 0x0FFF0000) << 40)));
        PML2 = *(PageMapLevel2Entry*)(&temp);
        PML2.Present = 1;
        PML2.Address = (uint64_t)g_PPFA->AllocatePage() >> 12;
        fast_memset((void*)(PML2.Address << 12), 0, 512);
        ((PageMapLevel2Entry*)((uint64_t)(PML3.Address) << 12))[pd] = PML2;
    }
    else {
        uint64_t temp = *(uint64_t*)(&PML2);
        temp |= flags & 0xFFF;
        temp |= (uint64_t)(flags & 0xFFF0000) << 40;
        ((PageMapLevel2Entry*)((uint64_t)(PML3.Address) << 12))[pd] = *(PageMapLevel2Entry*)&temp;
    }

    uint64_t temp = ((uint64_t)((flags & 0x0FFF) | ((uint64_t)(flags & 0x0FFF0000) << 40)));
    PageMapLevel1Entry PML1 = *(PageMapLevel1Entry*)(&temp);
    PML1.Address = (physical_addr >> 12);

    ((PageMapLevel1Entry*)((uint64_t)(PML2.Address) << 12))[pt] = PML1;
}

void x86_64_map_page(void* physaddr, void* virtualaddr, uint32_t flags) {
    x86_64_map_page_noflush(physaddr, virtualaddr, flags);
    x86_64_FlushTLB();
}

void x86_64_unmap_page(void* virtualaddr) {
    x86_64_unmap_page_noflush(virtualaddr);
    x86_64_FlushTLB();
}

void x86_64_unmap_page_noflush(void* virtualaddr) {
    uint64_t virtual_addr = (uint64_t)virtualaddr & ~0xFFF;

    const uint16_t pt    = (uint16_t)((virtual_addr & 0x0000001FF000) >> 12);
    const uint16_t pd    = (uint16_t)((virtual_addr & 0x00003FE00000) >> 21);
    const uint16_t pdptr = (uint16_t)((virtual_addr & 0x007FC0000000) >> 30);
    const uint16_t pml4  = (uint16_t)((virtual_addr & 0xFF8000000000) >> 39);

    PageMapLevel4Entry PML4 = PML4_Array.entries[pml4];
    if (PML4.Present == 0)
        return; // page isn't mapped

    PageMapLevel3Entry PML3 = ((PageMapLevel3Entry*)((uint64_t)(PML4.Address) << 12))[pdptr];
    if (PML3.Present == 0)
        return; // page isn't mapped

    PageMapLevel2Entry PML2 = ((PageMapLevel2Entry*)((uint64_t)(PML3.Address) << 12))[pd];
    if (PML2.Present == 0)
        return; // page isn't mapped

    fast_memset(&(((PageMapLevel1Entry*)((uint64_t)(PML2.Address) << 12))[pt]), 0, (sizeof(PageMapLevel1Entry) >> 3));
}

// Identity map memory. If length and/or start_phys aren't page aligned, the values used are rounded down to the nearest page boundary.
void x86_64_identity_map(void* start_phys, uint64_t length, uint32_t flags) {

    // ensure everything is page aligned
    length -= length % 4096;
    uint64_t real_phys = (uint64_t)start_phys;
    real_phys -= real_phys % 4096;

    // prepare the length
    length /= 4096;

    // actually map the memory
    for (uint64_t i = 0; i < length; i++) {
        x86_64_map_page((void*)(real_phys + (i * 4096)), (void*)(real_phys + (i * 4096)), flags);
    }
}

void x86_64_SetKernelAddress(void* kernel_virtual, void* kernel_physical, size_t length) {
    g_kernel_physical = kernel_physical;
    g_kernel_virtual = kernel_virtual;
    g_kernel_length = length;
}
