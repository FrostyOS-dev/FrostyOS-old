/*
Copyright (©) 2022-2023  Frosty515

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

#include "PagingInit.hpp"
#include "PageMapIndexer.hpp"

#include <Memory/PhysicalPageFrameAllocator.hpp>
#include <Memory/VirtualPageManager.hpp>

#include <HAL/hal.hpp>

#include "../ELFKernel.hpp"
#include "../Stack.h"
#include "../io.h"

#include <stdio.hpp>

WorldOS::PhysicalPageFrameAllocator PPFA;
WorldOS::VirtualPageManager KVPM;
WorldOS::VirtualRegion KVRegion;

size_t g_MemorySize = 0;

void x86_64_InitPaging(WorldOS::MemoryMapEntry** MemoryMap, uint64_t MMEntryCount, uint64_t kernel_virtual, uint64_t kernel_physical, size_t kernel_size, uint64_t fb_virt, uint64_t fb_size, uint64_t HHDM_start) {
    if (!x86_64_EnsureNX())
        WorldOS::Panic("No execute is not available. It is required.", nullptr, false);
    if (!x86_64_EnsureLargePages())
        WorldOS::Panic("2MiB pages are not available. They are required.", nullptr, false);

    if ((HHDM_start & ~0xffffff8000000000) > 0) // HHDM lower 39 bits have a value
        WorldOS::Panic("HHDM must have PML3, PML2, PML1 and offset values of 0.", nullptr, false);

    g_MemorySize = GetMemorySize((const WorldOS::MemoryMapEntry**)MemoryMap, MMEntryCount);

    x86_64_SetKernelAddress((void*)kernel_virtual, (void*)kernel_physical, kernel_size);
    x86_64_SetHHDMStart((void*)HHDM_start);

    // prepare page tables

    fast_memset(&PML4_Array, 0, sizeof(Level3Group) / 8);
    fast_memset(&PML3_LowestArray, 0, sizeof(Level2Group) / 8);
    fast_memset(&PML2_LowestArray, 0, sizeof(Level2Group) / 8);
    fast_memset(&PML1_LowestArray, 0, sizeof(Level1Group) / 8);
    fast_memset(&PML3_KernelGroup, 0, sizeof(Level2Group) / 8);
    fast_memset(&PML2_KernelLower, 0, sizeof(Level1Group) / 8);
    fast_memset(&PML1_KernelLowest, 0, sizeof(Level1Group) / 8);

    
    PML4_Array.entries[0].Present = 1;
    PML4_Array.entries[0].ReadWrite = 1;
    PML4_Array.entries[0].UserSuper = 1;
    PML4_Array.entries[0].Address = (uint64_t)x86_64_get_physaddr(&PML3_LowestArray) >> 12;

    PML4_Array.entries[511].Present = 1;
    PML4_Array.entries[511].ReadWrite = 1;
    PML4_Array.entries[511].Address = (uint64_t)x86_64_get_physaddr(&PML3_KernelGroup) >> 12;

    uint16_t HHDM_PML4_offset = (HHDM_start & 0x0000ff8000000000) >> 39;
    PML4_Array.entries[HHDM_PML4_offset].Present = 1;
    PML4_Array.entries[HHDM_PML4_offset].ReadWrite = 1;
    PML4_Array.entries[HHDM_PML4_offset].Address = (uint64_t)x86_64_get_physaddr(&PML3_LowestArray) >> 12;

    PML3_LowestArray.entries[0].Present = 1;
    PML3_LowestArray.entries[0].ReadWrite = 1;
    PML3_LowestArray.entries[0].UserSuper = 1;
    PML3_LowestArray.entries[0].Address = (uint64_t)x86_64_get_physaddr(&PML2_LowestArray) >> 12;

    PML3_KernelGroup.entries[510].Present = 1;
    PML3_KernelGroup.entries[510].ReadWrite = 1;
    PML3_KernelGroup.entries[510].Address = (uint64_t)x86_64_get_physaddr(&PML2_KernelLower) >> 12;

    PML2_LowestArray.entries[0].Present = 1;
    PML2_LowestArray.entries[0].ReadWrite = 1;
    PML2_LowestArray.entries[0].UserSuper = 1;
    PML2_LowestArray.entries[0].Address = (uint64_t)x86_64_get_physaddr(&PML1_LowestArray) >> 12;

    PML2_KernelLower.entries[0].Present = 1;
    PML2_KernelLower.entries[0].ReadWrite = 1;
    PML2_KernelLower.entries[0].Address = (uint64_t)x86_64_get_physaddr(&PML1_KernelLowest) >> 12;
    
    volatile uint64_t PML4_phys = (uint64_t)x86_64_get_physaddr(&PML4_Array);

    /* Create page map */

    if ((kernel_size & 4095) > 0) {
        kernel_size -= kernel_size & 4095;
        kernel_size += 4096;
    }
    
    x86_64_WorldOS::MapKernel((void*)kernel_physical, (void*)kernel_virtual, kernel_size); // do not flush TLB until after kernel is loaded

    // Perform early initialisation of physical MM
    PPFA.EarlyInit(MemoryMap[0], MMEntryCount, g_MemorySize);
    g_PPFA = &PPFA;


    // Map from end of 1st page until 2MiB
    for (uint64_t i = 0x1000; i < 0x200000; i += 0x1000)
        x86_64_map_page_noflush((void*)i, (void*)(i + HHDM_start), 0x8000003); // Read/Write, Present, Execute Disable
    
    
    // Map from 2MiB to 4GiB with 2MiB pages
    for (uint64_t i = 0x200000; i < 0x100000000; i += 0x200000)
        x86_64_map_large_page_noflush((void*)i, (void*)(i + HHDM_start), 0x8000003); // Read/Write, Present, Execute Disable

    for (uint64_t i = 0; i < MMEntryCount; i++) {
        WorldOS::MemoryMapEntry* entry = MemoryMap[i];
        if ((entry->Address + entry->length) < 0x100000000) continue; // skip entries that have already been mapped
        uint64_t addr = entry->Address & ~0xfff;
        uint64_t length = entry->length + 0xfff;
        length &= ~0xfff;
        
        for (uint64_t j = 0; j < length; j+=0x1000)
            x86_64_map_page_noflush((void*)(j + addr), (void*)(j + addr + HHDM_start), 0x8000003); // Read/Write, Present, Execute Disable
    }

    // Map the framebuffer
    fb_size += 4095;
    fb_size >>= 12; // avoid slow division

    uint64_t fb_phys = fb_virt - HHDM_start;

    for (uint64_t i = 0; i < fb_size; i++) {
        x86_64_map_page_noflush((void*)(fb_phys + i * 4096), (void*)(fb_virt + i * 4096), 0x8000003); // Present, Read/Write, Execute Disable
    }

    fb_size <<= 12; // convert back for later use
    
    /*
    Notes:
    - No page tables need to mapped as they are are inside the kernel and are already mapped.
    - If kernel goes over 2MB, extra set(s) of PML1 tables will need to be added
    - the stack does not need to be mapped as it is in the kernel file
    */

    volatile CR3Layout cr3_layout;
    fast_memset((void*)&cr3_layout, 0, sizeof(CR3Layout) / 8);
    cr3_layout.Address = PML4_phys >> 12;


    x86_64_LoadCR3(*((uint64_t*)&cr3_layout), kernel_physical); // will flush the TLB, so it does not need to be done earlier

    // must be done after PPFA is initialised so physical memory can be allocated for the page tables
    { // run inside new {} so so the WorldOS namespace is only "used" within that section of code
        using namespace WorldOS;
        for (uint64_t i = 0; i < MMEntryCount; i++) {
            MemoryMapEntry* entry = (MemoryMapEntry*)((uint64_t)MemoryMap + (i * MEMORY_MAP_ENTRY_SIZE));
            if (entry->type == WORLDOS_MEMORY_ACPI_NVS || entry->type == WORLDOS_MEMORY_ACPI_RECLAIMABLE) {
                uint64_t length = (entry->length % 4095 > 0 ? entry->length + 1 : entry->length);
                x86_64_identity_map((void*)(entry->Address), length, 0x3); // Present, Read/Write, Execute
            }
        }
    }

    // Fully initialise physical MM
    PPFA.FullInit(MemoryMap[0], MMEntryCount, g_MemorySize);
    g_PPFA = &PPFA;

    KVRegion = WorldOS::VirtualRegion((void*)(kernel_virtual + kernel_size), (void*)(~UINT64_C(0)));

    // Setup kernel virtual MM
    KVPM.InitVPageMgr(MemoryMap, MMEntryCount, (void*)kernel_virtual, kernel_size, (void*)fb_virt, fb_size, KVRegion);
    WorldOS::g_KVPM = &KVPM;
}