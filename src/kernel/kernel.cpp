#include "kernel.h"
#include "gdt.h"
#include "cstr.h"
#include "Bitmap.h"
#include "PageFrameAllocator.h"

namespace WorldOS {

    void InitKernel(void* FrameBufferAddress, uint64_t FrameBufferWidth, uint64_t FrameBufferHeight, MemoryMapEntry** MemoryMap, const size_t MemoryMapEntryCount) {
        Kernel kernel({FrameBufferAddress, FrameBufferWidth, FrameBufferHeight}, MemoryMap, MemoryMapEntryCount);
    }

    Kernel::Kernel(const FrameBuffer frameBuffer, MemoryMapEntry** MemoryMap, const size_t MemoryMapEntryCount) {
        m_fgcolour = 0xFFFFFFFF;
        m_bgcolour = 0;
        m_CursorPosition = {0,0};
        m_Stage = EARLY_STAGE;
        GDT* gdt = &DefaultGDT;
        GDTDescriptor gdtDescriptor = {(sizeof(GDT) - 1), ((uint64_t)gdt)};
        LoadGDT(&gdtDescriptor);
        uint64_t memSize = GetMemorySize(MemoryMap[0], MemoryMapEntryCount);

        m_InitialFrameBuffer = frameBuffer;
        fpu_init();

        PageFrameAllocator frameAllocator(MemoryMap[0], MemoryMapEntryCount);

        Print("Total System Memory: ");
        Print(to_string(memSize));
        Print("\nFree Memory: ");
        Print(to_string(frameAllocator.GetFreeMemory()));
        Print("\nUsed Memory: ");
        Print(to_string(frameAllocator.GetUsedMemory()));
        Print("\nReserved Memory: ");
        Print(to_string(frameAllocator.GetReservedMemory()));
        Print("\n");

        Print("Starting WorldOS!\n");
        /* Following code is temperary */
        
        while (true) {
            
        }
    }

    Kernel::~Kernel() {

    }

    void Kernel::Print(const char* message) {
        FrameBuffer* currentBuffer = nullptr;
        if (m_Stage == EARLY_STAGE) {
            currentBuffer = &m_InitialFrameBuffer;
        }
        char c = 0x00;
        uint64_t location = 0;
        while (true) {
            c = message[location++];
            if (c == 0x00) break;
            if (c == '\n') {
                m_CursorPosition = {0, m_CursorPosition.y + 16};
            }
            else {
                if (currentBuffer->FrameBufferWidth < (m_CursorPosition.x + 18)) {
                    m_CursorPosition = {0, m_CursorPosition.y + 16};
                }
                DrawChar(*currentBuffer, c, m_CursorPosition.x, m_CursorPosition.y, 0xFFFFFFFF, 0x0);
                m_CursorPosition = {m_CursorPosition.x + 10, m_CursorPosition.y};
            }
        }
    }
 
    void Kernel::Panic(uint64_t ExitCode) {

    }
}