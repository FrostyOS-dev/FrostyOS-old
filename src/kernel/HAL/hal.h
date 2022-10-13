#ifndef _KERNEL_HAL_H
#define _KERNEL_HAL_H

#include <arch/x86_64/panic.h>

#include <arch/x86_64/Graphics/graphics-defs.h>

namespace WorldOS {

    void HAL_Init(const FrameBuffer& fb);

    inline void Panic(const char* reason, x86_64_Registers* regs, const bool type = false) { x86_64_Panic(reason, regs, type); }

    void debug_putc(const char c);
    void debug_puts(const char* str);
}

#endif /* _KERNEL_HAL_H */