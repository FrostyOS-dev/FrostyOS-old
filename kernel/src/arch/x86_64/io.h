#ifndef _WORLDOS_KERNEL_X86_64_IO_h
#define _WORLDOS_KERNEL_X86_64_IO_h

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void x86_64_outb(uint16_t port, uint8_t value);
extern uint8_t x86_64_inb(uint16_t port);

extern void x86_64_EnableInterrupts();
extern void x86_64_DisableInterrupts();

extern void x86_64_iowait();

#ifdef __cplusplus
}
#endif

#endif /* _WORLDOS_KERNEL_X86_64_IO_h */