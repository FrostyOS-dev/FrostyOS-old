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

#include "panic.hpp"
#include "io.h"
#include "Stack.hpp"
#include "ELFSymbols.hpp"

#include "Scheduling/taskutil.hpp"

#include "interrupts/APIC/IPI.hpp"

#include <stdio.h>

#include <Graphics/VGA.hpp>

#include <tty/TTY.hpp>

#include <Scheduling/Scheduler.hpp>

BasicVGA* g_VGADevice;

static Colour g_panic_background;

void x86_64_SetPanicVGADevice(BasicVGA* device) {
    g_VGADevice = device;
    g_panic_background = Colour(device->GetBackgroundColour().GetFormat(), 0x1A, 0, 0xF7);
}

x86_64_Registers g_regs; // only used as buffer

char const* g_panic_reason = nullptr;

extern "C" void __attribute__((noreturn)) x86_64_Panic(const char* reason, void* data, const bool type) {
    x86_64_DisableInterrupts();

    x86_64_SendIPI(x86_64_GetCurrentLocalAPIC()->GetRegisters(), 0, x86_64_IPI_DeliveryMode::NMI, false, false, x86_64_IPI_DestinationShorthand::AllExcludingSelf, 0);
    
    Scheduling::Scheduler::Stop();

    //reason = "temp";
    if (reason == nullptr)
        reason = g_panic_reason;

    x86_64_Interrupt_Registers* i_regs = nullptr;
    x86_64_Registers* regs = nullptr;
    if (type) {
        i_regs = (x86_64_Interrupt_Registers*)data;
        x86_64_ConvertToStandardRegisters(&g_regs, i_regs);
        regs = &g_regs;
    }
    else
        regs = (x86_64_Registers*)data;

    // Output all to debug first

    dbgputs("KERNEL PANIC!\n");
    if (type)
        dbgputs("Exception: ");
    dbgprintf("%s\n", reason);

    dbgprintf("RAX=%016lx  RBX=%016lx  RCX=%016lx  RDX=%016lx\nRSI=%016lx  RDI=%016lx  RSP=%016lx  RBP=%016lx\nR8 =%016lx  R9 =%016lx  R10=%016lx  R11=%016lx\nR12=%016lx  R13=%016lx  R14=%016lx  R15=%016lx\nRIP=%016lx  RFL=%016lx", regs->RAX, regs->RBX, regs->RCX, regs->RDX, regs->RSI, regs->RDI, regs->RSP, regs->RBP, regs->R8, regs->R9, regs->R10, regs->R11, regs->R12, regs->R13, regs->R14, regs->R15, regs->RIP, regs->RFLAGS);
    dbgprintf("\nCS=%04hx  DS=%04hx", regs->CS, regs->DS);
    if (type /* true = interrupt */) {
        dbgprintf("  SS=%04hx\nINTERRUPT=%02hhx", i_regs->ss, i_regs->interrupt);
        if (i_regs->error != 0)
            dbgprintf("  ERROR CODE=%08x\n", i_regs->error);
        else
            dbgputc('\n');
        if (i_regs->interrupt == 0xE /* Page Fault */)
            dbgprintf("CR2=%016lx  CR3=%016lx\n", i_regs->CR2, regs->CR3);
    }
    else
        dbgputc('\n');

    dbgprintf("Stack trace:\n");
    char const* name = nullptr;
    if (g_KernelSymbols != nullptr)
        name = g_KernelSymbols->LookupSymbol(regs->RIP);
    dbgprintf("%016lx", regs->RIP);
    if (name != nullptr)
        dbgprintf(": %s\n", name);
    else
        dbgputc('\n');

    x86_64_walk_stack_frames((void*)(regs->RBP));

    dbgputs("\nThreads:\n");

    Scheduling::Scheduler::PrintThreads(stddebug);

    // Output all to stdout after in case framebuffer writes cause a page fault

    if (g_VGADevice == nullptr)
        dbgprintf("\nWARNING VGA Device unavailable.\n");
    else {
        g_VGADevice->ClearScreen(g_panic_background /* blue */);
        g_VGADevice->SetBackgroundColour(g_panic_background /* blue */);
        g_VGADevice->SetCursorPosition({0,0});
        g_VGADevice->SwapBuffers(false);

        g_CurrentTTY->SetVGADevice(g_VGADevice);

        puts("KERNEL PANIC!\n");
        if (type)
            puts("Exception: ");
        printf("%s\n", reason);

        printf("RAX=%016lx  RBX=%016lx  RCX=%016lx  RDX=%016lx\nRSI=%016lx  RDI=%016lx  RSP=%016lx  RBP=%016lx\nR8 =%016lx  R9 =%016lx  R10=%016lx  R11=%016lx\nR12=%016lx  R13=%016lx  R14=%016lx  R15=%016lx\nRIP=%016lx  RFL=%016lx", regs->RAX, regs->RBX, regs->RCX, regs->RDX, regs->RSI, regs->RDI, regs->RSP, regs->RBP, regs->R8, regs->R9, regs->R10, regs->R11, regs->R12, regs->R13, regs->R14, regs->R15, regs->RIP, regs->RFLAGS);
        printf("\nCS=%04hx  DS=%04hx", regs->CS, regs->DS);
        if (type /* true = interrupt */) {
            printf("  SS=%04hx\nINTERRUPT=%02hhx", i_regs->ss, i_regs->interrupt);
            if (i_regs->error != 0)
                printf("  ERROR CODE=%08x\n", i_regs->error);
            else
                putc('\n');
            if (i_regs->interrupt == 0xE /* Page Fault */)
                printf("CR2=%016lx  CR3=%016lx\n", i_regs->CR2, regs->CR3);
        }
    }

    while (true) {
        // just hang
        __asm__ volatile ("hlt");
    }
}