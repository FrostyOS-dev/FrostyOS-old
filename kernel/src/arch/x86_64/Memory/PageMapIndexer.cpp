#include "PageMapIndexer.hpp"

PageMapLevel4Entry __attribute__((aligned(0x1000))) g_PML4;
Level3Group __attribute__((aligned(0x1000))) PML3_Array;
Level2Group __attribute__((aligned(0x1000))) PML2_LowestArray;
Level1Group __attribute__((aligned(0x1000))) PML1_LowestArray;
Level2Group __attribute__((aligned(0x1000))) PML2_KernelGroup; // only highest 2 entries are used
Level1Group __attribute__((aligned(0x1000))) PML1_KernelLower;

void* g_kernel_physical = nullptr;
void* g_kernel_virtual  = nullptr;
size_t g_kernel_length = 0;

void* x86_64_get_physaddr(void* virtualaddr) {

    // check if there is a simpler way
    if ((uint64_t)virtualaddr <= (uint64_t)virtualaddr && ((uint64_t)virtualaddr + g_kernel_length) > (uint64_t)virtualaddr) {
        return (void*)((uint64_t)virtualaddr - (uint64_t)g_kernel_virtual + (uint64_t)g_kernel_physical);
    }

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

    uint16_t Page  = (uint16_t)(((uint64_t)virtualaddr & 0x0000001FF000) >> 12);
    uint16_t pt    = (uint16_t)(((uint64_t)virtualaddr & 0x00003FE00000) >> 21);
    uint16_t pd    = (uint16_t)(((uint64_t)virtualaddr & 0x007FC0000000) >> 30);
    uint16_t pdptr = (uint16_t)(((uint64_t)virtualaddr & 0xFF8000000000) >> 39);

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
