/*
Copyright (©) 2022-2024  Frosty515

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "PageMapIndexer.hpp"
#include "PagingUtil.hpp"

#include "../interrupts/APIC/IPI.hpp"

#include <Scheduling/Scheduler.hpp>

Level4Group __attribute__((aligned(0x1000))) K_PML4_Array;
void* g_KPML4_physical;

void* g_kernel_physical = nullptr;
void* g_kernel_virtual  = nullptr;
size_t g_kernel_length = 0;

void* g_HHDM_start = nullptr;

#include <stdio.h>
#include <util.h>

#include <Memory/PhysicalPageFrameAllocator.hpp>

void* x86_64_get_physaddr(Level4Group* PML4Array, void* virtualaddr) {

    // check if there is a simpler way
    if ((uint64_t)virtualaddr >= (uint64_t)g_kernel_virtual && ((uint64_t)g_kernel_virtual + g_kernel_length) > (uint64_t)virtualaddr) {
        return (void*)((uint64_t)virtualaddr - (uint64_t)g_kernel_virtual + (uint64_t)g_kernel_physical);
    }
    else if ((uint64_t)virtualaddr >= (uint64_t)g_HHDM_start && ((uint64_t)g_HHDM_start + GiB(4)) > (uint64_t)virtualaddr)
        return (void*)((uint64_t)virtualaddr - (uint64_t)g_HHDM_start);

    uint64_t virtualAddress = (uint64_t)virtualaddr;
    const uint16_t offset = (uint16_t)((virtualAddress & 0x000000000FFF) >>  0);
    const uint16_t PT_i   = (uint16_t)((virtualAddress & 0x0000001FF000) >> 12);
    const uint16_t PD_i   = (uint16_t)((virtualAddress & 0x00003FE00000) >> 21);
    const uint16_t PDP_i  = (uint16_t)((virtualAddress & 0x007FC0000000) >> 30);
    const uint16_t PML4_i = (uint16_t)((virtualAddress & 0xFF8000000000) >> 39);

    PageMapLevel4Entry PML4 = PML4Array->entries[PML4_i];
    if (PML4.Present == 0)
        return nullptr;

    PageMapLevel3Entry PML3 = (((PageMapLevel3Entry*)x86_64_to_HHDM((void*)((uint64_t)PML4.Address << 12)))[PDP_i]);
    if (PML3.Present == 0)
        return nullptr;

    PageMapLevel2Entry PML2 = (((PageMapLevel2Entry*)x86_64_to_HHDM((void*)((uint64_t)PML3.Address << 12)))[PD_i]);
    if (PML2.Present == 0)
        return nullptr;

    if (PML2.PageSize == 1) {
        PageMapLevel2Entry_LargePages PML2_Large = *(PageMapLevel2Entry_LargePages*)(&PML2);
        return (void*)(((uint64_t)(PML2_Large.Address) << 21) | (PT_i << 12) | offset);
    }

    PageMapLevel1Entry PML1 = (((PageMapLevel1Entry*)x86_64_to_HHDM((void*)((uint64_t)PML2.Address << 12)))[PT_i]);
    if (PML1.Present == 0)
        return nullptr;
    
    return (void*)(((uint64_t)PML1.Address << 12) | offset);
}

void* x86_64_to_HHDM(void* physaddr) {
    if (((uint64_t)physaddr < 0x1000))
        return nullptr;
    else if ((uint64_t)physaddr > (GiB(512) - 1))
        return physaddr;
    return (void*)((uint64_t)physaddr + (uint64_t)g_HHDM_start);
}

void x86_64_map_page_noflush(Level4Group* PML4Array, void* physaddr, void* virtualaddr, uint32_t flags) {
    uint64_t physical_addr = (uint64_t)physaddr & ~0xFFF;
    uint64_t virtual_addr = (uint64_t)virtualaddr & ~0xFFF;

    const uint16_t PT_i    = (uint16_t)((virtual_addr & 0x0000001FF000) >> 12);
    const uint16_t PD_i    = (uint16_t)((virtual_addr & 0x00003FE00000) >> 21);
    const uint16_t PDP_i   = (uint16_t)((virtual_addr & 0x007FC0000000) >> 30);
    const uint16_t PML4_i  = (uint16_t)((virtual_addr & 0xFF8000000000) >> 39);

    PageMapLevel4Entry PML4 = PML4Array->entries[PML4_i];
    if (PML4.Present == 0) {
        uint64_t temp = ((uint64_t)((flags & 0x0FFF) | ((uint64_t)(flags & 0x07FF0000) << 36)));
        PML4 = *(PageMapLevel4Entry*)(&temp);
        PML4.Present = 1;
        PML4.Address = (uint64_t)g_PPFA->AllocatePage() >> 12;
        memset(x86_64_to_HHDM((void*)(PML4.Address << 12)), 0, 4096);
        PML4Array->entries[PML4_i] = PML4;
    }
    else {
        uint64_t temp = *(uint64_t*)(&PML4);
        temp |= flags & 0xFFF;
        temp |= (uint64_t)(flags & 0x7FF0000) << 36;
        PML4Array->entries[PML4_i] = *(PageMapLevel4Entry*)&temp;
    }

    PageMapLevel3Entry PML3 = ((PageMapLevel3Entry*)x86_64_to_HHDM((void*)((uint64_t)(PML4.Address) << 12)))[PDP_i];
    if (PML3.Present == 0) {
        uint64_t temp = ((uint64_t)((flags & 0x0FFF) | ((uint64_t)(flags & 0x07FF0000) << 36)));
        PML3 = *(PageMapLevel3Entry*)(&temp);
        PML3.Present = 1;
        PML3.Address = (uint64_t)g_PPFA->AllocatePage() >> 12;
        memset(x86_64_to_HHDM((void*)(PML3.Address << 12)), 0, 4096);
        ((PageMapLevel3Entry*)x86_64_to_HHDM((void*)((uint64_t)(PML4.Address) << 12)))[PDP_i] = PML3;
    }
    else {
        uint64_t temp = *(uint64_t*)(&PML3);
        temp |= flags & 0xFFF;
        temp |= (uint64_t)(flags & 0x7FF0000) << 36;
        ((PageMapLevel3Entry*)x86_64_to_HHDM((void*)((uint64_t)(PML4.Address) << 12)))[PDP_i] = *(PageMapLevel3Entry*)&temp;
    }

    PageMapLevel2Entry PML2 = ((PageMapLevel2Entry*)x86_64_to_HHDM((void*)((uint64_t)(PML3.Address) << 12)))[PD_i];
    if (PML2.Present == 0) {
        uint64_t temp = ((uint64_t)((flags & 0x0FFF) | ((uint64_t)(flags & 0x07FF0000) << 36)));
        PML2 = *(PageMapLevel2Entry*)(&temp);
        PML2.Present = 1;
        PML2.Address = (uint64_t)g_PPFA->AllocatePage() >> 12;
        memset(x86_64_to_HHDM((void*)(PML2.Address << 12)), 0, 4096);
        ((PageMapLevel2Entry*)x86_64_to_HHDM((void*)((uint64_t)(PML3.Address) << 12)))[PD_i] = PML2;
    }
    else {
        uint64_t temp = *(uint64_t*)(&PML2);
        temp |= flags & 0xFFF;
        temp |= (uint64_t)(flags & 0x7FF0000) << 36;
       ((PageMapLevel2Entry*)x86_64_to_HHDM((void*)((uint64_t)(PML3.Address) << 12)))[PD_i] = *(PageMapLevel2Entry*)&temp;
    }
    if (PML2.PageSize)
        return; // Using 2MiB pages, stop

    uint64_t temp = ((uint64_t)((flags & 0x0FFF) | ((uint64_t)(flags & 0x0FFF0000) << 36)));
    PageMapLevel1Entry PML1 = *(PageMapLevel1Entry*)(&temp);
    PML1.Address = (physical_addr >> 12);

    ((PageMapLevel1Entry*)x86_64_to_HHDM((void*)((uint64_t)(PML2.Address) << 12)))[PT_i] = PML1;
}


void x86_64_map_page(Level4Group* PML4Array, void* physaddr, void* virtualaddr, uint32_t flags) {
    x86_64_map_page_noflush(PML4Array, physaddr, virtualaddr, flags);
    x86_64_FlushTLB();
}

void x86_64_unmap_page(Level4Group* PML4Array, void* virtualaddr) {
    x86_64_unmap_page_noflush(PML4Array, virtualaddr);
    x86_64_FlushTLB();
}

void x86_64_unmap_page_noflush(Level4Group* PML4Array, void* virtualaddr) {
    uint64_t virtual_addr = (uint64_t)virtualaddr & ~0xFFF;

    const uint16_t pt    = (uint16_t)((virtual_addr & 0x0000001FF000) >> 12);
    const uint16_t pd    = (uint16_t)((virtual_addr & 0x00003FE00000) >> 21);
    const uint16_t pdptr = (uint16_t)((virtual_addr & 0x007FC0000000) >> 30);
    const uint16_t pml4  = (uint16_t)((virtual_addr & 0xFF8000000000) >> 39);

    PageMapLevel4Entry* PML4 = &(PML4Array->entries[pml4]);
    if (PML4->Present == 0)
        return; // page isn't mapped

    PageMapLevel3Entry* PML3 = &(((PageMapLevel3Entry*)x86_64_to_HHDM((void*)((uint64_t)(PML4->Address) << 12)))[pdptr]);
    if (PML3->Present == 0)
        return; // page isn't mapped

    PageMapLevel2Entry* PML2 = &(((PageMapLevel2Entry*)x86_64_to_HHDM((void*)((uint64_t)(PML3->Address) << 12)))[pd]);
    if (PML2->Present == 0 || PML2->PageSize)
        return; // page isn't mapped or is a 2MiB page

    memset(&(((PageMapLevel1Entry*)x86_64_to_HHDM((void*)((uint64_t)(PML2->Address) << 12)))[pt]), 0, sizeof(PageMapLevel1Entry));

    Level1Group* group = (Level1Group*)x86_64_to_HHDM((void*)((uint64_t)(PML2->Address) << 12));
    bool used = false;
    for (uint_fast16_t i = 0; i < 512; i++) {
        if (group->entries[i].Present == 1) {
            used = true;
            break;
        }
    }
    if (!used) {
        g_PPFA->FreePage(group);
        PML2->Present = 0;
    }
    Level2Group* group2 = (Level2Group*)x86_64_to_HHDM((void*)((uint64_t)(PML3->Address) << 12));
    used = false;
    for (uint_fast16_t i = 0; i < 512; i++) {
        if (group2->entries[i].Present == 1) {
            used = true;
            break;
        }
    }
    if (!used) {
        g_PPFA->FreePage(group2);
        PML3->Present = 0;
    }
    Level3Group* group3 = (Level3Group*)x86_64_to_HHDM((void*)((uint64_t)(PML4->Address) << 12));
    used = false;
    for (uint_fast16_t i = 0; i < 512; i++) {
        if (group3->entries[i].Present == 1) {
            used = true;
            break;
        }
    }
    if (!used) {
        g_PPFA->FreePage(group3);
        PML4->Present = 0;
    }
}

// Update flags of page mapping
void x86_64_remap_page(Level4Group* PML4Array, void* virtualaddr, uint32_t flags) {
    x86_64_remap_page_noflush(PML4Array, virtualaddr, flags);
    x86_64_FlushTLB();
}

// Update flags of page mapping with no TLB flush
void x86_64_remap_page_noflush(Level4Group* PML4Array, void* virtualaddr, uint32_t flags) {
    uint64_t virtual_addr = (uint64_t)virtualaddr & ~0xFFF;

    const uint16_t pt    = (uint16_t)((virtual_addr & 0x0000001FF000) >> 12);
    const uint16_t pd    = (uint16_t)((virtual_addr & 0x00003FE00000) >> 21);
    const uint16_t pdptr = (uint16_t)((virtual_addr & 0x007FC0000000) >> 30);
    const uint16_t pml4  = (uint16_t)((virtual_addr & 0xFF8000000000) >> 39);

    PageMapLevel4Entry PML4 = PML4Array->entries[pml4];
    if (PML4.Present == 0)
        return;
    else {
        uint64_t temp = *(uint64_t*)(&PML4);
        temp |= flags & 0xFFF;
        temp |= (uint64_t)(flags & 0x7FF0000) << 36;
        PML4Array->entries[pml4] = *(PageMapLevel4Entry*)&temp;
    }

    PageMapLevel3Entry PML3 = ((PageMapLevel3Entry*)x86_64_to_HHDM((void*)((uint64_t)(PML4.Address) << 12)))[pdptr];
    if (PML3.Present == 0)
        return;
    else {
        uint64_t temp = *(uint64_t*)(&PML3);
        temp |= flags & 0xFFF;
        temp |= (uint64_t)(flags & 0x7FF0000) << 36;
        ((PageMapLevel3Entry*)x86_64_to_HHDM((void*)((uint64_t)(PML4.Address) << 12)))[pdptr] = *(PageMapLevel3Entry*)&temp;
    }

    PageMapLevel2Entry PML2 = ((PageMapLevel2Entry*)x86_64_to_HHDM((void*)((uint64_t)(PML3.Address) << 12)))[pd];
    if (PML2.Present == 0)
        return;
    else {
        uint64_t temp = *(uint64_t*)(&PML2);
        temp |= flags & 0xFFF;
        temp |= (uint64_t)(flags & 0x7FF0000) << 36;
       ((PageMapLevel2Entry*)x86_64_to_HHDM((void*)((uint64_t)(PML3.Address) << 12)))[pd] = *(PageMapLevel2Entry*)&temp;
    }
    if (PML2.PageSize)
        return; // Using 2MiB pages, stop

    PageMapLevel1Entry PML1 = ((PageMapLevel1Entry*)x86_64_to_HHDM((void*)((uint64_t)(PML2.Address) << 12)))[pt];
    uint64_t temp = ((uint64_t)((flags & 0x0FFF) | ((uint64_t)(flags & 0x0FFF0000) << 36)));
    temp |= (PML1.Address << 12) & 0x000FFFFFFFFFF000;
    ((uint64_t*)x86_64_to_HHDM((void*)((uint64_t)(PML2.Address) << 12)))[pt] = temp;
}

// Identity map memory. If length and/or start_phys aren't page aligned, the values used are rounded down to the nearest page boundary.
void x86_64_identity_map(Level4Group* PML4Array, void* start_phys, uint64_t length, uint32_t flags) {

    // ensure everything is page aligned
    length -= length % 4096;
    uint64_t real_phys = (uint64_t)start_phys;
    real_phys -= real_phys % 4096;

    // prepare the length
    length /= 4096;

    // actually map the memory
    for (uint64_t i = 0; i < length; i++) {
        x86_64_map_page(PML4Array, (void*)(real_phys + (i * 4096)), (void*)(real_phys + (i * 4096)), flags);
    }
}

void x86_64_map_large_page_noflush(Level4Group* PML4Array, void* physaddr, void* virtualaddr, uint32_t flags) {
    uint64_t physical_addr = (uint64_t)physaddr & ~0x1FFFFF;
    uint64_t virtual_addr = (uint64_t)virtualaddr & ~0x1FFFFF;

    const uint16_t pd    = (uint16_t)((virtual_addr & 0x00003FE00000) >> 21);
    const uint16_t pdptr = (uint16_t)((virtual_addr & 0x007FC0000000) >> 30);
    const uint16_t pml4  = (uint16_t)((virtual_addr & 0xFF8000000000) >> 39);

    PageMapLevel4Entry PML4 = PML4Array->entries[pml4];
    if (PML4.Present == 0) {
        uint64_t temp = ((uint64_t)((flags & 0x0FFF) | ((uint64_t)(flags & 0x07FF0000) << 36)));
        PML4 = *(PageMapLevel4Entry*)(&temp);
        PML4.Present = 1;
        PML4.Address = (uint64_t)g_PPFA->AllocatePage() >> 12;
        memset(x86_64_to_HHDM((void*)(PML4.Address << 12)), 0, 4096);
        PML4Array->entries[pml4] = PML4;
    }
    else {
        uint64_t temp = *(uint64_t*)(&PML4);
        temp |= flags & 0xFFF;
        temp |= (uint64_t)(flags & 0x7FF0000) << 36;
        PML4Array->entries[pml4] = *(PageMapLevel4Entry*)&temp;
    }

    PageMapLevel3Entry PML3 = ((PageMapLevel3Entry*)x86_64_to_HHDM((void*)((uint64_t)(PML4.Address) << 12)))[pdptr];
    if (PML3.Present == 0) {
        uint64_t temp = ((uint64_t)((flags & 0x0FFF) | ((uint64_t)(flags & 0x07FF0000) << 36)));
        PML3 = *(PageMapLevel3Entry*)(&temp);
        PML3.Present = 1;
        PML3.Address = (uint64_t)g_PPFA->AllocatePage() >> 12;
        memset(x86_64_to_HHDM((void*)(PML3.Address << 12)), 0, 4096);
        ((PageMapLevel3Entry*)x86_64_to_HHDM((void*)((uint64_t)(PML4.Address) << 12)))[pdptr] = PML3;
    }
    else {
        uint64_t temp = *(uint64_t*)(&PML3);
        temp |= flags & 0xFFF;
        temp |= (uint64_t)(flags & 0x7FF0000) << 36;
        ((PageMapLevel3Entry*)x86_64_to_HHDM((void*)((uint64_t)(PML4.Address) << 12)))[pdptr] = *(PageMapLevel3Entry*)&temp;
    }

    uint64_t temp = ((uint64_t)((flags & 0x1FFF) | ((uint64_t)(flags & 0x0FFF0000) << 36))); // allow extra flags bit for large page mappings
    PageMapLevel2Entry_LargePages PML2 = *(PageMapLevel2Entry_LargePages*)(&temp);
    PML2.Address = (physical_addr >> 21);
    PML2.PageSize = 1;

    ((PageMapLevel2Entry_LargePages*)x86_64_to_HHDM((void*)((uint64_t)(PML3.Address) << 12)))[pd] = PML2;
}

void x86_64_map_large_page(Level4Group* PML4Array, void* physaddr, void* virtualaddr, uint32_t flags) {
    x86_64_map_large_page_noflush(PML4Array, physaddr, virtualaddr, flags);
    x86_64_FlushTLB();
}

void x86_64_unmap_large_page_noflush(Level4Group* PML4Array, void* virtualaddr) {
    uint64_t virtual_addr = (uint64_t)virtualaddr & ~0x1FFFFF;

    const uint16_t pd    = (uint16_t)((virtual_addr & 0x00003FE00000) >> 21);
    const uint16_t pdptr = (uint16_t)((virtual_addr & 0x007FC0000000) >> 30);
    const uint16_t pml4  = (uint16_t)((virtual_addr & 0xFF8000000000) >> 39);

    PageMapLevel4Entry* PML4 = &(PML4Array->entries[pml4]);
    if (PML4->Present == 0)
        return; // page isn't mapped

    PageMapLevel3Entry* PML3 = &(((PageMapLevel3Entry*)x86_64_to_HHDM((void*)((uint64_t)(PML4->Address) << 12)))[pdptr]);
    if (PML3->Present == 0)
        return; // page isn't mapped

    memset(&(((PageMapLevel2Entry*)x86_64_to_HHDM((void*)((uint64_t)(PML3->Address) << 12)))[pd]), 0, sizeof(PageMapLevel2Entry));

    Level2Group* group2 = (Level2Group*)x86_64_to_HHDM((void*)((uint64_t)(PML3->Address) << 12));
    bool used = false;
    for (uint_fast16_t i = 0; i < 512; i++) {
        if (group2->entries[i].Present == 1) {
            used = true;
            break;
        }
    }
    if (!used) {
        g_PPFA->FreePage(group2);
        PML3->Present = 0;
    }
    Level3Group* group3 = (Level3Group*)x86_64_to_HHDM((void*)((uint64_t)(PML4->Address) << 12));
    used = false;
    for (uint_fast16_t i = 0; i < 512; i++) {
        if (group3->entries[i].Present == 1) {
            used = true;
            break;
        }
    }
    if (!used) {
        g_PPFA->FreePage(group3);
        PML3->Present = 0;
    }
}

void x86_64_unmap_large_page(Level4Group* PML4Array, void* virtualaddr) {
    x86_64_unmap_large_page_noflush(PML4Array, virtualaddr);
    x86_64_FlushTLB();
}

// Update flags of page mapping
void x86_64_remap_large_page(Level4Group* PML4Array, void* virtualaddr, uint32_t flags) {
    x86_64_remap_page_noflush(PML4Array, virtualaddr, flags);
    x86_64_FlushTLB();
}

// Update flags of page mapping with no TLB flush
void x86_64_remap_large_page_noflush(Level4Group* PML4Array, void* virtualaddr, uint32_t flags) {
    uint64_t virtual_addr = (uint64_t)virtualaddr & ~0x1FFFFF;

    const uint16_t pd    = (uint16_t)((virtual_addr & 0x00003FE00000) >> 21);
    const uint16_t pdptr = (uint16_t)((virtual_addr & 0x007FC0000000) >> 30);
    const uint16_t pml4  = (uint16_t)((virtual_addr & 0xFF8000000000) >> 39);

    PageMapLevel4Entry PML4 = PML4Array->entries[pml4];
    if (PML4.Present == 0)
        return;
    else {
        uint64_t temp = *(uint64_t*)(&PML4);
        temp |= flags & 0xFFF;
        temp |= (uint64_t)(flags & 0x7FF0000) << 36;
        PML4Array->entries[pml4] = *(PageMapLevel4Entry*)&temp;
    }

    PageMapLevel3Entry PML3 = ((PageMapLevel3Entry*)x86_64_to_HHDM((void*)((uint64_t)(PML4.Address) << 12)))[pdptr];
    if (PML3.Present == 0)
        return;
    else {
        uint64_t temp = *(uint64_t*)(&PML3);
        temp |= flags & 0xFFF;
        temp |= (uint64_t)(flags & 0x7FF0000) << 36;
        ((PageMapLevel3Entry*)x86_64_to_HHDM((void*)((uint64_t)(PML4.Address) << 12)))[pdptr] = *(PageMapLevel3Entry*)&temp;
    }

    PageMapLevel2Entry PML2 = ((PageMapLevel2Entry*)x86_64_to_HHDM((void*)((uint64_t)(PML3.Address) << 12)))[pd];
    uint64_t temp = ((uint64_t)((flags & 0x0FFF) | ((uint64_t)(flags & 0x0FFF0000) << 36)));
    temp |= (PML2.Address << 12) & 0x000FFFFFFFFFF000;
    ((uint64_t*)x86_64_to_HHDM((void*)((uint64_t)(PML3.Address) << 12)))[pd] = temp;
}

void x86_64_SetKernelAddress(void* kernel_virtual, void* kernel_physical, size_t length) {
    g_kernel_physical = kernel_physical;
    g_kernel_virtual = kernel_virtual;
    g_kernel_length = length;
}

void x86_64_SetHHDMStart(void* virtualaddr) {
    g_HHDM_start = virtualaddr;
}

void* x86_64_GetHHDMStart() {
    return g_HHDM_start;
}

void x86_64_TLBShootdown(void *address, uint64_t length, bool wait) {
    if (!Scheduling::Scheduler::GlobalIsRunning())
        return x86_64_InvalidatePages((uint64_t)address, length);
    x86_64_IPI_TLBShootdown shootdown = {(uint64_t)address, length};
    x86_64_IssueIPI(x86_64_IPI_DestinationShorthand::AllIncludingSelf, 0, x86_64_IPI_Type::TLBShootdown, (uint64_t)&shootdown, wait);
}
