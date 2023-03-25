#include "PageTableManager.hpp"
#include "PageMapIndexer.hpp"

#include <Memory/PhysicalPageFrameAllocator.hpp>
#include <Memory/VirtualPageManager.hpp>

#include "../ELFKernel.hpp"

#include <HAL/hal.hpp>

#include <stdio.hpp>

#include <arch/x86_64/Stack.h>

#include <HAL/hal.hpp>

#include <arch/x86_64/io.h>

namespace x86_64_WorldOS {

    WorldOS::PhysicalPageFrameAllocator PPFA;
    WorldOS::VirtualPageManager KVPM;

    /* PageTableManager class */

    /* Public Methods */
    
    PageTableManager::PageTableManager(WorldOS::MemoryMapEntry** MemoryMap, uint64_t MMEntryCount, uint64_t kernel_virtual, uint64_t kernel_physical, size_t kernel_size, volatile uint64_t fb_phys, volatile uint64_t fb_virt, volatile uint64_t fb_size) {
        if (!x86_64_EnsureNX())
            WorldOS::Panic("No execute is not available. It is required.", nullptr, false);
        
        m_MemorySize = GetMemorySize((const WorldOS::MemoryMapEntry**)MemoryMap, MMEntryCount);

        x86_64_SetKernelAddress((void*)kernel_virtual, (void*)kernel_physical, kernel_size);

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
        
        volatile uint64_t PML4_phys = (volatile uint64_t)x86_64_get_physaddr(&PML4_Array);

        /* Create page map */

        if (kernel_size % 4096 > 0) {
            kernel_size -= kernel_size % 4096;
            kernel_size += 4096;
        }
        
        MapKernel((void*)kernel_physical, (void*)kernel_virtual, kernel_size); // do not flush TLB until after kernel is loaded

        
        PML1_LowestArray.entries[0].Present = 1;
        PML1_LowestArray.entries[0].ReadWrite = 1;
        PML1_LowestArray.entries[0].Address = 0;
        

        /*
        Notes:
        - No page tables need to mapped as they are are inside the kernel and are already mapped.
        - If kernel goes over 2MB, extra set of PML1 will need to be added
        - the stack does not need to be mapped as it is in the kernel file
        */

        volatile CR3Layout cr3_layout;
        fast_memset((void*)&cr3_layout, 0, sizeof(CR3Layout) / 8);
        cr3_layout.Address = PML4_phys >> 12;


        x86_64_LoadCR3(*((uint64_t*)&cr3_layout), kernel_physical); // will flush the TLB, so it does not need to be done earlier
        
        // Initialise physical MM
        PPFA.SetMemoryMap(MemoryMap[0], MMEntryCount);
        g_PPFA = &PPFA;

        // must be done after PPFA is initialised so physical memory can be allocated for the page tables
        { // run inside new {} so so the WorldOS namespace is only "used" within that section of code
            using namespace WorldOS;
            for (uint64_t i = 0; i < MMEntryCount; i++) {
                MemoryMapEntry* entry = (MemoryMapEntry*)((uint64_t)MemoryMap + (i * MEMORY_MAP_ENTRY_SIZE));
                if (entry->type == WORLDOS_MEMORY_ACPI_NVS || entry->type == WORLDOS_MEMORY_ACPI_RECLAIMABLE) {
                    uint64_t length = (entry->length % 4096 > 0 ? entry->length + 1 : entry->length);
                    x86_64_identity_map((void*)(entry->Address), length, 0x3); // Present, Read/Write, Execute
                }
            }
        }

        fb_size += 4095;
        fb_size /= 4096;

        for (uint64_t i = 0; i < fb_size; i++) {
            x86_64_map_page((void*)(fb_phys + i * 4096), (void*)(fb_virt + i * 4096), 0x8000003); // Present, Read/Write, Execute Disable
        }

        // Setup kernel virtual MM
        KVPM.InitVPageMgr(*MemoryMap, MMEntryCount, (void*)kernel_virtual, kernel_size, (void*)fb_virt, fb_size, (void*)(kernel_virtual+kernel_size), ~(uint64_t)0);
    }

    PageTableManager::~PageTableManager() {
        WorldOS::Panic("PageTableManager exited.", nullptr, false);
    }
}