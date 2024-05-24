/*
Copyright (©) 2022-2024  Frosty515

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

#include "../Scheduling/taskutil.hpp"

#include <stdio.h>
#include <util.h>

#include <HAL/hal.hpp>

#include <Memory/PageFault.hpp>

#include <Scheduling/Thread.hpp>
#include <Scheduling/Process.hpp>
#include <Scheduling/Scheduler.hpp>

x86_64_ISRHandler_t g_ISRHandlers[256];

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

extern "C" void x86_64_ISR_Handler(x86_64_Interrupt_Registers* regs) {
    //dbgprintf("ISR: %d\n", regs->interrupt);

    /* Check if there is a designated handler */
    if (g_ISRHandlers[regs->interrupt] != nullptr)
        return g_ISRHandlers[regs->interrupt](regs);

    if (regs->interrupt == 0x0E) { // Page fault
        PageFaultErrorCode error_code;
        error_code.readable = regs->error & 0x1;
        error_code.writable = regs->error & 0x2;
        error_code.user = regs->error & 0x4;
        error_code.reserved_write = regs->error & 0x8;
        error_code.instruction_fetch = regs->error & 0x10;
        x86_64_Registers real_regs;
        x86_64_ConvertToStandardRegisters(&real_regs, regs);
        PageFaultHandler(error_code, (void*)regs->CR2, (void*)regs->rip, &real_regs);
    }
    else if (regs->interrupt == 0x00 || regs->interrupt == 0x06 || regs->interrupt == 0x0D || regs->interrupt == 0x13) { // Divide by zero, Invalid opcode, General protection fault, SIMD Floating-Point Exception
        Scheduling::Thread* thread = Scheduling::Scheduler::GetCurrent();
        Scheduling::Process* process = nullptr;
        Scheduling::Priority priority = Scheduling::Priority::KERNEL;
        if (thread != nullptr) {
            x86_64_Registers real_regs;
            x86_64_ConvertToStandardRegisters(&real_regs, regs);
            fast_memcpy(thread->GetCPURegisters(), &real_regs, sizeof(x86_64_Registers)); // save the registers
            process = thread->GetParent();
            if (process != nullptr)
                priority = process->GetPriority();
        }
        if (Scheduling::Scheduler::isRunning() && process != nullptr && priority != Scheduling::Priority::KERNEL) {
            int signum;
            switch (regs->interrupt) {
            case 0x00:
                signum = SIGFPE;
                break;
            case 0x06:
                signum = SIGILL;
                break;
            case 0x0D:
                signum = SIGSEGV;
                break;
            case 0x13:
                signum = SIGFPE;
                break;
            default:
                signum = SIGILL;
                break;
            }
            process->ReceiveSignal(signum);
        }
    }

    dbgprintf("Interrupt occurred. RIP: %016lx Interrupt number: %02hhx\n", regs->rip, regs->interrupt);

    // prevent spam panic messages
    if (in_interrupt) {
        __asm__ volatile ("cli");
        while (true)
            __asm__ volatile ("hlt");
    }

    /* TEMP */
    char tempReason[64];
    memset(tempReason, 0, 64);

    const size_t strLength = g_ExceptionsSTRLengths[regs->interrupt];

    memcpy(tempReason, g_Exceptions[regs->interrupt], strLength);

    in_interrupt = true;
    x86_64_Panic(tempReason, regs, true);


    while (true); // just hang
}