#ifndef _WORLDOS_KERNEL_X86_64_IO_h
#define _WORLDOS_KERNEL_X86_64_IO_h

#include "../../wos-stddef.h"

void __attribute__((cdecl)) x86_64_outb(uint16_t port, uint8_t value);
uint8_t __attribute__((cdecl)) x86_64_inb(uint16_t port);

void __attribute__((cdecl)) x86_64_EnableInterrupts();
void __attribute__((cdecl)) x86_64_DisableInterrupts();

inline void x86_64_iowait() {
    x86_64_outb(0x80, 0);
}

#endif /* _WORLDOS_KERNEL_X86_64_IO_h */