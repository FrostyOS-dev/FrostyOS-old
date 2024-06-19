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
#include "arch/x86_64/Memory/PAT.hpp"

#include <stdio.h>
#include <util.h>

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

    memset(&K_PML4_Array, 0, sizeof(K_PML4_Array));
    
    volatile uint64_t PML4_phys = (uint64_t)x86_64_get_physaddr(&K_PML4_Array, &K_PML4_Array);
    g_KPML4_physical = (void*)PML4_phys;

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
    for (uint64_t i = 0x1000; i < 0x200000; i += 0x1000)
        x86_64_map_page_noflush(&K_PML4_Array, (void*)i, (void*)(i + HHDM_start), 0x8000003); // Read/Write, Present, Execute Disable
    
    uint64_t fb_phys = fb_virt - HHDM_start;

    uint64_t largePagePart0End = ALIGN_DOWN((uint64_t)fb_phys, 0x200000);

    uint64_t fbLargePagesStart = ALIGN_UP((uint64_t)fb_phys, 0x200000);
    uint64_t fbLargePagesEnd = ALIGN_DOWN(((uint64_t)fb_phys + fb_size), 0x200000);
    
    // Map from 2MiB to before start of framebuffer with 2MiB pages
    for (uint64_t i = 0x200000; i < largePagePart0End && i < GiB(4); i += 0x200000)
        x86_64_map_large_page_noflush(&K_PML4_Array, (void*)i, (void*)(i + HHDM_start), 0x8000003); // Read/Write, Present, Execute Disable

    // Map remaining pages until framebuffer
    for (uint64_t i = largePagePart0End; i < fb_phys && i < GiB(4); i += 0x1000)
        x86_64_map_page_noflush(&K_PML4_Array, (void*)i, (void*)(i + HHDM_start), 0x8000003); // Read/Write, Present, Execute Disable
    
    // Map framebuffer until 2MiB boundary as write combining
    for (uint64_t i = fb_phys; i < fbLargePagesStart; i += 0x1000)
        x86_64_map_page_noflush(&K_PML4_Array, (void*)i, (void*)(i + HHDM_start), 0x8000003 | x86_64_GetPTEFlagsFromPAT(x86_64_PATType::WriteCombining)); // Read/Write, Present, Execute Disable

    // Map framebuffer with 2MiB pages until end of alignment as write combining
    for (uint64_t i = fbLargePagesStart; i < fbLargePagesEnd; i += 0x200000)
        x86_64_map_large_page_noflush(&K_PML4_Array, (void*)i, (void*)(i + HHDM_start), 0x8000003 | x86_64_GetPDEFlagsFromPAT(x86_64_PATType::WriteCombining)); // Read/Write, Present, Execute Disable

    // Map remaining framebuffer pages as write combining
    for (uint64_t i = fbLargePagesEnd; i < (fb_phys + fb_size); i += 0x1000)
        x86_64_map_page_noflush(&K_PML4_Array, (void*)i, (void*)(i + HHDM_start), 0x8000003 | x86_64_GetPTEFlagsFromPAT(x86_64_PATType::WriteCombining)); // Read/Write, Present, Execute Disable

    if ((fb_phys + fb_size) < 0x100000000) { // if framebuffer is not in HHDM address space
        // Map from end of framebuffer until next 2MiB boundary with normal pages
        for (uint64_t i = fb_phys + fb_size; i < ALIGN_DOWN((uint64_t)fb_phys + fb_size, 0x200000) && i < GiB(4); i += 0x1000)
            x86_64_map_page_noflush(&K_PML4_Array, (void*)i, (void*)(i + HHDM_start), 0x8000003); // Read/Write, Present, Execute Disable

        // Map from 2MiB boundary after end of framebuffer until 4GiB with 2MiB pages
        for (uint64_t i = ALIGN_UP((uint64_t)fb_phys + fb_size, 0x200000); i < GiB(4); i += 0x200000)
            x86_64_map_large_page_noflush(&K_PML4_Array, (void*)i, (void*)(i + HHDM_start), 0x8000003); // Read/Write, Present, Execute Disable
    }

    for (uint64_t i = 0; i < MMEntryCount; i++) {
        MemoryMapEntry* entry = MemoryMap[i];
        if ((entry->Address + entry->length) < 0x100000000) continue; // skip entries that have already been mapped
        uint64_t addr = ALIGN_DOWN(entry->Address, PAGE_SIZE);
        uint64_t length = ALIGN_UP(entry->length, PAGE_SIZE);

        // we break up the mapping into 3 sections: pre-2MiB alignment, 2MiB pages, and post-2MiB alignment

        // Map from start of entry until 2MiB boundary with normal pages
        for (uint64_t j = addr; j < (ALIGN_UP((addr + length), (MiB(2)))); j += PAGE_SIZE)
            x86_64_map_page_noflush(&K_PML4_Array, (void*)j, (void*)(j + HHDM_start), 0x8000003); // Read/Write, Present, Execute Disable
        
        // Map from 2MiB boundary after end of entry until next 2MiB boundary with 2MiB pages
        for (uint64_t j = ALIGN_UP((addr + length), (MiB(2))); j < ALIGN_DOWN((addr + length), (MiB(2))); j += MiB(2))
            x86_64_map_large_page_noflush(&K_PML4_Array, (void*)j, (void*)(j + HHDM_start), 0x8000003); // Read/Write, Present, Execute Disable

        // Map from 2MiB boundary after end of entry until end of entry with normal pages
        for (uint64_t j = ALIGN_DOWN((addr + length), (MiB(2))); j < (addr + length); j += PAGE_SIZE)
            x86_64_map_page_noflush(&K_PML4_Array, (void*)j, (void*)(j + HHDM_start), 0x8000003); // Read/Write, Present, Execute Disable
    }
    
    /*
    Notes:
    - No page tables need to mapped as they are are inside the kernel and are already mapped.
    - If kernel goes over 2MB, extra set(s) of PML1 tables will need to be added
    - the stack does not need to be mapped as it is in the kernel file
    - framebuffer is mapped as WC (write combining) to improve performance
    */

    volatile CR3Layout cr3_layout;
    memset((void*)&cr3_layout, 0, sizeof(CR3Layout));
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
