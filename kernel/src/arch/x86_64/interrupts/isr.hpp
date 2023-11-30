/*
Copyright (©) 2022-2023  Frosty515

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _KERNEL_ISR_HPP
#define _KERNEL_ISR_HPP

#include <stdint.h>

#include "IDT.hpp"

struct x86_64_Interrupt_Registers {
    uint64_t ds;                                         // data segment pushed by us
    uint64_t CR3, CR2;                                   // CR3 and CR2 pushed by us. useful for memory related exceptions
    uint64_t R15, R14, R13, R12, R11, R10, R9, R8, RDI, RSI, RBP, RSP, RDX, RBX, RCX, RAX; // pushaq macro
    uint64_t interrupt, error;                           // we push interrupt, error is pushed automatically
    uint64_t rip, cs, rflags, rsp, ss;                   // pushed automatically by CPU
} __attribute__((packed));


typedef void (*x86_64_ISRHandler_t)(x86_64_Interrupt_Registers* regs);

void x86_64_ISR_Initialize();
void x86_64_ISR_RegisterHandler(uint8_t interrupt, x86_64_ISRHandler_t handler);

extern "C" void x86_64_ISR_Handler(x86_64_Interrupt_Registers* regs);

void x86_64_ISR_InitializeGates();

#endif /* _KERNEL_INTERRUPTS_HPP */