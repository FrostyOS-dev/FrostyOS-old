#include "kernel.hpp"

#include <arch/x86_64/ELFSymbols.h>

#include <Memory/PageManager.hpp>
#include <Memory/kmalloc.hpp>

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

        HAL_EarlyInit(params->MemoryMap, params->MemoryMapEntryCount, params->kernel_virtual_addr, params->kernel_physical_addr, kernel_size, params->hhdm_start_addr, m_InitialFrameBuffer);
        KPM.InitPageManager((void*)(params->kernel_virtual_addr + kernel_size), UINT64_MAX - (params->kernel_virtual_addr + kernel_size), false);
        g_KPM = &KPM;
        kmalloc_init();

        if (params->frameBuffer.bpp != 32) {
            Panic("Bootloader Frame Buffer Bits per Pixel is not 32", nullptr, false);
        }

        HAL_Stage2(params->RSDP_table);

        fprintf(VFS_DEBUG_AND_STDOUT, "Starting WorldOS!\n");

        // hang
        while (true) {
            
        }
    }
}
