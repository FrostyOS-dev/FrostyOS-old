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

#include "isr.hpp"
#include <HAL/hal.hpp>

#define MODULE "ISR"

x86_64_ISRHandler_t g_ISRHandlers[256];

#include <stdio.hpp>

static const char* const g_Exceptions[] = {
    "Divide by 0",
    "Reserved",
    "Non-maskable Interrupt",
    "Breakpoint (INT3)",
    "Overflow (INTO)",
    "Bounds range exceeded",
    "Invalid opcode (UD2)",
    "Device not available (WAIT/FWAIT)",
    "Double fault",
    "Coprocessor segment overrun",
    "Invalid TSS",
    "Segment not present",
    "Stack-segment fault",
    "General protection fault",
    "Page fault",
    "Reserved",
    "x87 FPU error",
    "Alignment check",
    "Machine check",
    "SIMD Floating-Point Exception"
};

static const size_t g_ExceptionsSTRLengths[] = {
    12, 9, 23, 18, 16, 22, 21, 34, 13, 28, 12, 20, 20, 25, 11, 9, 14, 16, 14, 30
};

void x86_64_ISR_Initialize() {
    x86_64_ISR_InitializeGates();
    for (int i = 0; i < 256; i++)
        x86_64_IDT_EnableGate(i);
}

void x86_64_ISR_RegisterHandler(uint8_t interrupt, x86_64_ISRHandler_t handler) {
    g_ISRHandlers[interrupt] = handler;
    x86_64_IDT_EnableGate(interrupt);
}

bool in_interrupt = false;

extern "C" void x86_64_ISR_Handler(x86_64_Interrupt_Registers regs) {
    x86_64_Interrupt_Registers* p_regs = &regs;

    /* Check if there is a designated handler */
    if (g_ISRHandlers[p_regs->interrupt] != nullptr)
        return g_ISRHandlers[p_regs->interrupt](p_regs);

    fprintf(VFS_DEBUG, "Interrupt occurred. RIP: %lx Interrupt number: %hhx\n", regs.rip, regs.interrupt);

    // prevent spam panic messages
    if (in_interrupt) {
        __asm__ volatile ("cli");
        while (true)
            __asm__ volatile ("hlt");
    }

    /* TEMP */
    char tempReason[64];
    memset(tempReason, 0, 64);

    const size_t strLength = g_ExceptionsSTRLengths[p_regs->interrupt];

    memcpy(tempReason, g_Exceptions[p_regs->interrupt], strLength);

    in_interrupt = true;
    WorldOS::Panic(tempReason, p_regs, true);


    while (true); // just hang
}