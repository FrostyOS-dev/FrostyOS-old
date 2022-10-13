#ifndef _WORLDOS_KERNEL_X86_64_IO_h
#define _WORLDOS_KERNEL_X86_64_IO_h

#include <wos-stddef.h>
#include <wos-stdint.h>

extern "C" inline void x86_64_outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}
extern "C" inline uint8_t x86_64_inb(uint16_t port) {
    uint8_t returnVal;
    asm volatile ("inb %1, %0"
    : "=a"(returnVal)
    : "Nd"(port));
    return returnVal;
}

extern "C" inline void x86_64_EnableInterrupts() {
    asm volatile("sti");
}
extern "C" inline void x86_64_DisableInterrupts() {
    asm volatile("cli");
}

extern "C" inline void x86_64_iowait() {
    // 0x80 is an unused port, so we can use it to wait for a device to finish communicating on the IO bus
    x86_64_outb(0x80, 0);
}

#endif /* _WORLDOS_KERNEL_X86_64_IO_h */