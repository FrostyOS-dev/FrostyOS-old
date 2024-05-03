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
#include "PagingUtil.hpp"

#include <Memory/PhysicalPageFrameAllocator.hpp>
#include <Memory/VirtualPageManager.hpp>

#include <HAL/hal.hpp>

#include "../ELFKernel.hpp"

#include <stdio.h>

PhysicalPageFrameAllocator PPFA;
VirtualPageManager KVPM;
VirtualPageManager VPM;
VirtualRegion KVRegion;
VirtualRegion VAddressSpace;

size_t g_MemorySize = 0;

void x86_64_InitPaging(MemoryMapEntry** MemoryMap, uint64_t MMEntryCount, uint64_t kernel_virtual, uint64_t kernel_physical, size_t kernel_size, uint64_t fb_virt, uint64_t fb_size, uint64_t HHDM_start) {
    if (!x86_64_EnsureNX()) {
        PANIC("No execute is not available. It is required.");
    }
    if (!x86_64_EnsureLargePages()) {
        PANIC("2MiB pages are not available. They are required.");
    }

    if ((HHDM_start & ~0xffffff8000000000) > 0) { // HHDM lower 39 bits have a value
        PANIC("HHDM must have PML3, PML2, PML1 and offset values of 0.");
    }

    g_MemorySize = GetMemorySize((const MemoryMapEntry**)MemoryMap, MMEntryCount);

    x86_64_SetKernelAddress((void*)kernel_virtual, (void*)kernel_physical, kernel_size);
    x86_64_SetHHDMStart((void*)HHDM_start);
    
    volatile uint64_t PML4_phys = (uint64_t)x86_64_get_physaddr(&K_PML4_Array, &K_PML4_Array);

    // Perform early initialisation of physical MM
    PPFA.EarlyInit(MemoryMap[0], MMEntryCount, g_MemorySize);
    g_PPFA = &PPFA;

    /* Create page map */

    if ((kernel_size & 4095) > 0) {
        kernel_size -= kernel_size & 4095;
        kernel_size += 4096;
    }

    MapKernel((void*)kernel_physical, (void*)kernel_virtual, kernel_size); // do not flush TLB until after kernel is loaded

    // Map from end of 1st page until 2MiB
    for (uint64_t i = PAGE_SIZE; i < MiB(2); i += PAGE_SIZE)
        x86_64_map_page_noflush(&K_PML4_Array, (void*)i, (void*)(i + HHDM_start), 0x8000003); // Read/Write, Present, Execute Disable
    
    
    // Map from 2MiB to 4GiB with 2MiB pages
    for (uint64_t i = MiB(2); i < GiB(4); i += MiB(2))
        x86_64_map_large_page_noflush(&K_PML4_Array, (void*)i, (void*)(i + HHDM_start), 0x8000003); // Read/Write, Present, Execute Disable

    for (uint64_t i = 0; i < MMEntryCount; i++) {
        MemoryMapEntry* entry = MemoryMap[i];
        if ((entry->Address + entry->length) <= GiB(4)) continue; // skip entries that have already been mapped
        uint64_t addr = ALIGN_DOWN(entry->Address, PAGE_SIZE);
        uint64_t length = ALIGN_UP(entry->length, PAGE_SIZE);

        uint64_t large_page_length = ALIGN_DOWN(length, MiB(2));

        for (uint64_t j = 0; j < large_page_length; j += MiB(2))
            x86_64_map_large_page_noflush(&K_PML4_Array, (void*)(j + addr), (void*)(j + addr + HHDM_start), 0x8000003); // Read/Write, Present, Execute Disable
        
        length -= large_page_length;

        for (uint64_t j = 0; j < length; j += PAGE_SIZE)
            x86_64_map_page_noflush(&K_PML4_Array, (void*)(j + addr), (void*)(j + addr + HHDM_start), 0x8000003); // Read/Write, Present, Execute Disable
    }

    // Map the framebuffer
    fb_size += 4095;
    fb_size >>= 12; // avoid slow division

    uint64_t fb_phys = fb_virt - HHDM_start;

    for (uint64_t i = 0; i < fb_size; i++)
        x86_64_map_page_noflush(&K_PML4_Array, (void*)(fb_phys + i * 4096), (void*)(fb_virt + i * 4096), 0x8000003); // Present, Read/Write, Execute Disable

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

    x86_64_LoadCR3(*((uint64_t*)&cr3_layout)); // will flush the TLB, so it does not need to be done earlier

    // Fully initialise physical MM
    PPFA.FullInit(MemoryMap[0], MMEntryCount, g_MemorySize);
    g_PPFA = &PPFA;

    KVRegion = VirtualRegion((void*)(kernel_virtual + kernel_size), (void*)(~UINT64_C(0)));
    VAddressSpace = VirtualRegion((void*)0, (void*)(~UINT64_C(0)));

    // Setup kernel virtual MM
    VPM.InitVPageMgr(VAddressSpace);
    VPM.ReservePages(nullptr, 0x3FFFE0000000); // reserve first page, user process memory (0x1000-0x7FFFFFFFFFFF), and non-canonical address space (0x800000000000-0xFFFF7FFFFFFFFFFF)
    VPM.ReservePages((void*)HHDM_start, 0x100000000); // Reserve from HHDM_start to HHDM_start + 0x100000000000
    VPM.ReservePages((void*)kernel_virtual, DIV_ROUNDUP((KVRegion.GetSize() + kernel_size), 0x1000)); // reserve kernel address space
    VPM.ReservePages((void*)fb_virt, DIV_ROUNDUP(fb_size, 0x1000)); // reserve framebuffer (most likely in HHDM address space)
    g_VPM = &VPM;
    KVPM.InitVPageMgr(MemoryMap, MMEntryCount, (void*)kernel_virtual, kernel_size, (void*)fb_virt, fb_size, KVRegion);
    g_KVPM = &KVPM;
}