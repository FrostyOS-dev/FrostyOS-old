#include "kernel.h"
#include "HAL/hal.h"
#include "cstr.h"
#include "Bitmap.h"

namespace WorldOS {

    Kernel::Kernel(KernelParams params) {
        m_fgcolour = 0xFFFFFFFF;
        m_bgcolour = 0;
        m_Stage = EARLY_STAGE;
        HAL_Init();

        m_InitialFrameBuffer = params.frameBuffer;
        m_BasicRenderer = BasicRenderer(m_InitialFrameBuffer, {0,0}, m_fgcolour, m_bgcolour);
        GlobalBasicRenderer = &m_BasicRenderer;

        if (params.frameBuffer.bpp != 32) {
            Panic("Bootloader Frame Buffer Bits per Pixel is not 32", nullptr, false);
        }

        if (sizeof(char) != 1) {
            Panic("Size of char is not 1 byte", nullptr, false);
        }

        const uint64_t memSize = GetMemorySize((const MemoryMapEntry**)params.MemoryMap, params.MemoryMapEntryCount);

        m_BasicRenderer.ClearScreen(m_bgcolour);

        m_BasicRenderer.Print("Starting WorldOS!\n");

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

        int i = *(int*)0;
        i = 1;
        
        while (true) {
            
        }
    }

    Kernel::~Kernel() {

    }
}