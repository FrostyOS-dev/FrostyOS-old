#include "kernel.hpp"

#include <HAL/graphics.hpp>

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

        const uint64_t memSize = GetMemorySize((const MemoryMapEntry**)params.MemoryMap, params.MemoryMapEntryCount);

        VGA_ClearScreen(m_bgcolour);

        fprintf(VFS_DEBUG_AND_STDOUT, "Starting WorldOS!\n");

        // temp

        

        // hang
        while (true) {
            
        }
    }
}