#ifndef _KERNEL_H
#define _KERNEL_H

#include "wos-stdint.h"
#include "wos-stddef.h"
//include "Memory.h"
#include "graphics.h"

namespace WorldOS {

    void InitKernel(void* FrameBufferAddress, const uint64_t FrameBufferWidth, const uint64_t FrameBufferHeight/*, const MemoryMapEntry** memoryMap, const size_t MemoryMapEntryCount*/);

    struct Position {
        uint64_t x;
        uint64_t y;
    };

    class Kernel {
    public:
        Kernel(const FrameBuffer frameBuffer/*, const MemoryMapEntry** MemoryMap, const size_t MemoryMapEntryCount*/);
        ~Kernel();

        void Print(const char* message);

    private:
        void Panic(uint64_t ExitCode);

    private:
        FrameBuffer m_InitialFrameBuffer;
        uint32_t m_fgcolour;
        uint32_t m_bgcolour;
        Position m_CursorPosition;
    };

    inline void fpu_init() {
        uint64_t t;

        asm("clts");
        asm("mov %%cr0, %0" : "=r"(t));
        t &= ~(1 << 2);
        t |= (1 << 1);
        asm("mov %0, %%cr0" :: "r"(t));
        asm("mov %%cr4, %0" : "=r"(t));
        t |= 3 << 9;
        asm("mov %0, %%cr4" :: "r"(t));
        asm("fninit");
    }
}

#endif /* _KERNEL_H */