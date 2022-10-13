#include "kernel.h"

#include <HAL/graphics.h>

#include <arch/x86_64/E9.h>

namespace WorldOS {

    Kernel::Kernel(KernelParams params) {
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



        x86_64_debug_putc('M');
        

        /* Following code is temperary */

        /*m_BasicRenderer.Print("Displaying Memory Map!\n");

        for (uint64_t i = 0; i < params.MemoryMapEntryCount; i++) {
            MemoryMapEntry* entry = params.MemoryMap[i];
            m_BasicRenderer.Print("Address: 0x");
            m_BasicRenderer.Print(to_hstring(entry->Address));
            m_BasicRenderer.Print("   Length: ");
            m_BasicRenderer.Print(to_string(entry->length));
            m_BasicRenderer.Print("   Type: ");
            m_BasicRenderer.Print(to_string(entry->type));
            m_BasicRenderer.NewLine(); // same as printing \n
        }*/
        
        while (true) {
            
        }
    }

    Kernel::~Kernel() {

    }
}