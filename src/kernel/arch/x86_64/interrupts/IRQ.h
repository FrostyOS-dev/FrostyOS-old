#ifndef _KERNEL_X86_64_IRQ_H
#define _KERNEL_X86_64_IRQ_H

#include "isr.h"

typedef void (*x86_64_IRQHandler)(x86_64_Registers* regs);

void x86_64_IRQ_Initialize();
void x86_64_IRQ_RegisterHandler(const uint8_t irq, x86_64_IRQHandler handler);

#endif /* _KERNEL_X86_64_IRQ_H */