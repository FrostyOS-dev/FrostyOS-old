#include "PageTableManager.hpp"
#include "PageFrameAllocator.hpp"
#include "PageMapIndexer.hpp"

#include "../ELFKernel.hpp"

#include <HAL/hal.hpp>

#include <stdio.hpp>

namespace x86_64_WorldOS {

    PageFrameAllocator PFA;

    /* PageTableManager class */

    /* Public Methods */
    
    PageTableManager::PageTableManager(WorldOS::MemoryMapEntry** MemoryMap, uint64_t MMEntryCount, uint64_t kernel_virtual, uint64_t kernel_physical, size_t kernel_size) {
        m_MemorySize = GetMemorySize((const WorldOS::MemoryMapEntry**)MemoryMap, MMEntryCount);

        x86_64_SetKernelAddress((void*)kernel_virtual, (void*)kernel_physical, kernel_size);

        // Set temporary PML4

        uint64_t tempValue = x86_64_GetCR3();
        CR3Layout* layout = (CR3Layout*)&tempValue;

        g_PML4 = *(PageMapLevel4Entry*)(layout->Address << 12);

        // prepare page tables

        fast_memset(&PML3_Array, 0, sizeof(Level3Group) / 8);
        fast_memset(&PML2_LowestArray, 0, sizeof(Level2Group) / 8);
        fast_memset(&PML1_LowestArray, 0, sizeof(Level1Group) / 8);
        fast_memset(&PML2_KernelGroup, 0, sizeof(Level2Group) / 8);
        fast_memset(&PML1_KernelLower, 0, sizeof(Level1Group) / 8);

        
        PML3_Array.entries[0].Present = 1;
        PML3_Array.entries[0].Address = (uint64_t)x86_64_get_physaddr(&PML2_LowestArray) >> 12;

        PML2_LowestArray.entries[0].Present = 1;
        PML2_LowestArray.entries[0].Address = (uint64_t)x86_64_get_physaddr(&PML1_LowestArray) >> 12;

        PML2_KernelGroup.entries[510].Present = 1;
        PML2_KernelGroup.entries[510].Address = (uint64_t)x86_64_get_physaddr(&PML1_KernelLower) >> 12;
        
        // order looks weird, but is necessary so the bootloaders page tables are saved for as long as possible
        uint64_t PML3_addr = (uint64_t)x86_64_get_physaddr(&PML3_Array) >> 12;
        fast_memset(&g_PML4, 0, sizeof(PageMapLevel4Entry) / 8);
        g_PML4.Present = 1;
        g_PML4.Address = (uint64_t)x86_64_get_physaddr(&PML3_Array) >> 12;

        /* Create page map */
        x86_64_identity_map((void*)0, (m_MemorySize <= GiB(1) ? m_MemorySize : GiB(1)), 0x8000003); // Present, Read/Write, Execute Disabled

        if (kernel_size % 4096 > 0) {
            kernel_size -= kernel_size % 4096;
            kernel_size += 4096;
        }
        
        MapKernel((void*)kernel_physical, (void*)kernel_virtual, kernel_size);

        if ((uint64_t)&g_PML4 % 4096 != 0) {
            WorldOS::Panic("PML4 is not page-aligned!", nullptr, false);
        }

        x86_64_LoadCR3((uint64_t)&g_PML4);
        
        PFA.SetMemoryMap(MemoryMap[0], MMEntryCount);
    }

    PageTableManager::~PageTableManager() {
        WorldOS::Panic("PageTableManager exited.", nullptr, false);
    }
}