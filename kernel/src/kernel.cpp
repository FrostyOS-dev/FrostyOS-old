#include "kernel.hpp"

#include <HAL/graphics.hpp>

#include <arch/x86_64/Memory/PageTableManager.hpp>

#include <HAL/timer.hpp>

#include <arch/x86_64/Memory/PagingUtil.hpp>

namespace WorldOS {

    FrameBuffer m_InitialFrameBuffer;
    uint32_t m_fgcolour;
    uint32_t m_bgcolour;
    uint64_t m_Stage;

    extern "C" void StartKernel(KernelParams params) {
        m_fgcolour = 0xFFFFFFFF;
        m_bgcolour = 0;
        m_Stage = EARLY_STAGE;
        m_InitialFrameBuffer = params.frameBuffer;

        HAL_Init(m_InitialFrameBuffer);

        if (params.frameBuffer.bpp != 32) {
            Panic("Bootloader Frame Buffer Bits per Pixel is not 32", nullptr, false);
        }

        if (sizeof(char) != 1) {
            Panic("Size of char is not 1 byte", nullptr, false);
        }

        const volatile uint64_t memSize = GetMemorySize((const MemoryMapEntry**)params.MemoryMap, params.MemoryMapEntryCount);

        //VGA_ClearScreen(m_bgcolour);

        fprintf(VFS_DEBUG_AND_STDOUT, "Starting WorldOS!\n");

        // temp

        
        /*
        uint64_t fb_physical = 0;

        if ((uint64_t)(params.frameBuffer.FrameBufferAddress) & (UINT64_MAX - UINT32_MAX) == (uint64_t)(params.hhdm_start_addr)) {
            fb_physical = (uint64_t)(params.frameBuffer.FrameBufferAddress) - (uint64_t)(params.hhdm_start_addr);
        }
        else {
            Panic("Framebuffer is not in HHDM range!", nullptr, false);
        }

        fprintf(VFS_DEBUG, "Framebuffer physical: %x\n", fb_physical);

        x86_64_WorldOS::PageTableManager PTM(params.MemoryMap, params.MemoryMapEntryCount, params.kernel_virtual_addr, params.kernel_physical_addr, params.kernel_size, fb_physical, (uint64_t)(params.frameBuffer.FrameBufferAddress), VGA_GetScreenSizeBytes());

        
        fprintf(VFS_DEBUG, "PAGE TABLE MANAGER SUCCESS!!!\n");
        */
        
        fprintf(VFS_DEBUG_AND_STDOUT, "HHDM addr: %x\n", (uint64_t)(params.hhdm_start_addr));
        

        // hang
        while (true) {
            
        }
    }
}