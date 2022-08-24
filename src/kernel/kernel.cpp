#include "kernel.h"
#include "gdt.h"

namespace WorldOS {

    void InitKernel(void* FrameBufferAddress, uint64_t FrameBufferWidth, uint64_t FrameBufferHeight/*, const MemoryMapEntry** MemoryMap, const size_t MemoryMapEntryCount*/) {
        Kernel kernel({FrameBufferAddress, FrameBufferWidth, FrameBufferHeight}/*, MemoryMap, MemoryMapEntryCount*/);
    }

    Kernel::Kernel(const FrameBuffer frameBuffer/*, const MemoryMapEntry** MemoryMap, const size_t MemoryMapEntryCount*/) {
        m_fgcolour = 0xFFFFFFFF;
        m_bgcolour = 0;
        m_CursorPosition = {0,0};
        GDT* gdt = &DefaultGDT;
        GDTDescriptor gdtDescriptor = {(sizeof(GDT) - 1), ((uint64_t)gdt)};
        LoadGDT(&gdtDescriptor);

        m_InitialFrameBuffer = frameBuffer;
        fpu_init();

        /* Following code is temperary */
        const char message[15] = "Hello, World!\0";
        Print(message);
        while (true) {
            
        }
    }

    Kernel::~Kernel() {

    }

    void Kernel::Print(const char* message) {
        char c = 0x00;
        uint64_t location = 0;
        while (true) {
            c = message[location++];
            if (c == 0x00) break;
            if (c == '\n') {
                m_CursorPosition = {0, m_CursorPosition.y + 16};
            }
            else {
                DrawChar(m_InitialFrameBuffer, c, m_CursorPosition.x, m_CursorPosition.y, 0xFFFFFFFF, 0x0);
                m_CursorPosition = {m_CursorPosition.x + 8, m_CursorPosition.y};
            }
        }
    }
 
    void Kernel::Panic(uint64_t ExitCode) {

    }
}