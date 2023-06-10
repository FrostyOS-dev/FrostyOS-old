#ifndef _KERNEL_ISR_HPP
#define _KERNEL_ISR_HPP

#include <stdint.h>

#include "IDT.hpp"

struct x86_64_Interrupt_Registers {
    uint64_t ds;                                         // data segment pushed by us
    uint64_t CR2;                                        // CR2 pushed by us. useful for page faults
    uint64_t R15, R14, R13, R12, R11, R10, R9, R8, RDI, RSI, RBP, RSP, RDX, RBX, RCX, RAX; // pushaq macro
    uint64_t interrupt, error;                           // we push interrupt, error is pushed automatically
    uint64_t rip, cs, rflags, rsp, ss;                   // pushed automatically by CPU
} __attribute__((packed));

typedef void (*x86_64_ISRHandler_t)(x86_64_Interrupt_Registers* regs);

void x86_64_ISR_Initialize();
void x86_64_ISR_RegisterHandler(uint8_t interrupt, x86_64_ISRHandler_t handler);

extern "C" void x86_64_ISR_Handler(x86_64_Interrupt_Registers regs);

void x86_64_ISR_InitializeGates();

#endif /* _KERNEL_INTERRUPTS_HPP */