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

#include "E9.h"
#include "ELFSymbols.hpp"
#include "arch/x86_64/Processor.hpp"
#include "arch/x86_64/interrupts/APIC/LocalAPIC.hpp"
#include "io.h"
#include "panic.hpp"
#include "Stack.hpp"

#include "interrupts/APIC/IPI.hpp"

#include "Scheduling/taskutil.hpp"

#include <stdio.h>

#include <fs/FileDescriptorManager.hpp>

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

    if (x86_64_GetCurrentLocalAPIC() != nullptr)
        x86_64_IssueIPI(x86_64_IPI_DestinationShorthand::AllExcludingSelf, 0, x86_64_IPI_Type::Stop, 0, true);
    
    Scheduling::Scheduler::Stop();

    /* we have to force unlock all the kernel file descriptors so we can actually print debug info. */

    if (g_KFDManager != nullptr)
        g_KFDManager->ForceUnlock();
    else // this means we have a very early boot panic. This should not happen under any condition.
        x86_64_debug_puts("Kernel panic occurred and the Kernel File Descriptor Manager is unavailable.\nNo further info can be printed.\n");

    // Now we force unlock all the scheduler data structures

    Scheduling::Scheduler::ForceUnlockEverything();


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

    // Get Processor Info
    Scheduling::Scheduler::ProcessorInfo* info = GetCurrentProcessorInfo();
    uint8_t id = 255;
    uint8_t LAPIC_id = 255;
    if (info == nullptr)
        puts("WARNING: Processor info unavailable.\n");
    else {
        id = info->id;
        Processor* proc = info->processor;
        if (proc != nullptr) {
            x86_64_LocalAPIC* LAPIC = proc->GetLocalAPIC();
            if (LAPIC != nullptr)
                LAPIC_id = LAPIC->GetID();
        }
    }


    // Output all to debug first

    dbgputs("KERNEL PANIC!\n");
    if (type)
        dbgputs("Exception: ");
    dbgputs(reason);
    if (id != 255) {
        dbgprintf(" on CPU %hhu", id);
        if (LAPIC_id != 255)
            dbgprintf(" (LAPIC %hhu)", LAPIC_id);
    }
    dbgputc('\n');


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
        //g_VGADevice->ClearScreen(g_panic_background /* blue */);
        //g_VGADevice->SetBackgroundColour(g_panic_background /* blue */);
        //g_VGADevice->SetCursorPosition({0,0});
        //g_VGADevice->SwapBuffers(false);

        g_CurrentTTY->SetVGADevice(g_VGADevice);

        puts("KERNEL PANIC!\n");
        if (type)
            puts("Exception: ");
        puts(reason);
        if (id != 255) {
            printf(" on CPU %hhu", id);
            if (LAPIC_id != 255)
                printf(" (LAPIC %hhu)", LAPIC_id);
        }
        putc('\n');

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
        else
            putc('\n');

        printf("Stack trace:\n");
        char const* name = nullptr;
        if (g_KernelSymbols != nullptr)
            name = g_KernelSymbols->LookupSymbol(regs->RIP);
        printf("%016lx", regs->RIP);
        if (name != nullptr)
            printf(": %s\n", name);
        else
            putc('\n');

        x86_64_walk_stack_frames((void*)(regs->RBP));
    }

    while (true) {
        // just hang
        __asm__ volatile ("hlt");
    }
}