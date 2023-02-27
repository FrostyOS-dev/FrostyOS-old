#ifndef _KERNEL_X86_64_IRQ_HPP
#define _KERNEL_X86_64_IRQ_HPP

#include "isr.hpp"

typedef void (*x86_64_IRQHandler_t)(x86_64_Registers* regs);

void x86_64_IRQ_Initialize();
void x86_64_IRQ_RegisterHandler(const uint8_t irq, x86_64_IRQHandler_t handler);

#endif /* _KERNEL_X86_64_IRQ_HPP */