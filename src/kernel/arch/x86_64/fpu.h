#ifndef _KERNEL_X86_64_FPU_H
#define _KERNEL_X86_64_FPU_H

#include <stdint.h>

inline void x86_64_FPU_Initialize() {
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

#endif /* _KERNEL_X86_64_FPU_H */