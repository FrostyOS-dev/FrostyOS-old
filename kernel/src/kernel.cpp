#include "kernel.hpp"

#include <HAL/graphics.hpp>
#include <HAL/timer.hpp>

#include <arch/x86_64/ELFSymbols.h>

#include <Memory/PageManager.hpp>

namespace WorldOS {

    FrameBuffer m_InitialFrameBuffer;
    uint32_t m_fgcolour;
    uint32_t m_bgcolour;
    uint64_t m_Stage;

    PageManager KPM;

    extern "C" void StartKernel(KernelParams* params) {
        m_fgcolour = 0xFFFFFFFF;
        m_bgcolour = 0;
        m_Stage = EARLY_STAGE;
        m_InitialFrameBuffer = params->frameBuffer;

        uint64_t kernel_size = (uint64_t)_kernel_end_addr - (uint64_t)_text_start_addr;

        HAL_Init(params->MemoryMap, params->MemoryMapEntryCount, params->kernel_virtual_addr, params->kernel_physical_addr, kernel_size, params->hhdm_start_addr, m_InitialFrameBuffer);
        KPM.InitPageManager((void*)(params->kernel_virtual_addr + kernel_size), UINT64_MAX - (params->kernel_virtual_addr + kernel_size), false);
        g_KPM = &KPM;

        if (params->frameBuffer.bpp != 32) {
            Panic("Bootloader Frame Buffer Bits per Pixel is not 32", nullptr, false);
        }

        if (sizeof(char) != 1) {
            Panic("Size of char is not 1 byte", nullptr, false);
        }

        const volatile uint64_t memSize = GetMemorySize((const MemoryMapEntry**)params->MemoryMap, params->MemoryMapEntryCount);

        fprintf(VFS_DEBUG, "Kernel PARAMS:\nFrameBuffer: {Address=%lx Width=%lu Height=%lu bpp=%u}\nMemoryMap: %lx MemoryMapEntryCount: %lu\nEFI_SYSTEM_TABLE_ADDR: %lp\nkernel_physical_addr: %lx kernel_virtual_addr: %lx kernel_size: %lu\nRSDP_table: %lp\nhhdm_start_addr: %lx\n", params->frameBuffer.FrameBufferAddress, params->frameBuffer.FrameBufferWidth, params->frameBuffer.FrameBufferHeight, params->frameBuffer.bpp, params->MemoryMap, params->MemoryMapEntryCount, params->EFI_SYSTEM_TABLE_ADDR, params->kernel_physical_addr, params->kernel_virtual_addr, params->kernel_size, params->RSDP_table, params->hhdm_start_addr);

        fprintf(VFS_DEBUG_AND_STDOUT, "Starting WorldOS!\n");

        for (size_t i = 0; i < params->MemoryMapEntryCount; i++) {
            MemoryMapEntry* entry = params->MemoryMap[i];
            fprintf(VFS_DEBUG, "%lu: Entry Address: %lp Entry: {Address: %lx Length: %lu Type: %lu}\n", i, entry, entry->Address, entry->length, entry->type);
        }
        

        // hang
        while (true) {
            
        }
    }
}
