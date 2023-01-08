#ifndef _KERNEL_HAL_HPP
#define _KERNEL_HAL_HPP

#include <arch/x86_64/panic.hpp>

#include <arch/x86_64/Graphics/graphics-defs.h>

namespace WorldOS {

    void HAL_Init(const FrameBuffer& fb);

    // reason = message to display, regs = registers at the time of error, type = the type of error (true for interrupt and false for other)
    inline void Panic(const char* reason, x86_64_Registers* regs, const bool type = false) { x86_64_Panic(reason, regs, type); }
}

#endif /* _KERNEL_HAL_HPP */