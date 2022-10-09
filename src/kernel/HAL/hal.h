#ifndef _KERNEL_HAL_H
#define _KERNEL_HAL_H

#include <arch/x86_64/panic.h>

typedef x86_64_PanicArgs Global_PanicArgs;

namespace WorldOS {
    void HAL_Init();

    inline void Panic(const char* reason, x86_64_Registers* regs, const bool type = false) { x86_64_Panic(reason, regs, type); }
}

#endif /* _KERNEL_HAL_H */