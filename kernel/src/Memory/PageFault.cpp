/*
Copyright (©) 2024  Frosty515

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

#include "PageFault.hpp"

#include <stdio.h>

#include <Scheduling/Scheduler.hpp>

#ifdef __x86_64__
#include <arch/x86_64/ELFSymbols.hpp>
#include <arch/x86_64/Stack.hpp>
#endif

void __attribute__((noreturn)) PageFaultHandler(PageFaultErrorCode error_code, void* faulting_address, void* current_address, CPU_Registers* regs, BasicVGA* VGADevice, Colour& background) {
    Scheduling::Thread* thread = Scheduling::Scheduler::GetCurrent();
    Scheduling::Process* process = nullptr;
    if (thread != nullptr) {
        fast_memcpy(thread->GetCPURegisters(), regs, sizeof(CPU_Registers)); // save the registers
        process = thread->GetParent();
    }
    if (error_code.user && Scheduling::Scheduler::isRunning() && process != nullptr)
        process->ReceiveSignal(SIGSEGV);
    
    Scheduling::Scheduler::Stop();

    // this is not how it should be done, but we don't have sprintf/snprintf yet, so we don't have a better option

    dbgputs("KERNEL PANIC!\n");

    dbgprintf("Page fault in %s-mode at %lp while trying to %s a %s page at address %lp\n", error_code.user ? "user" : "kernel", current_address, error_code.writable ? "write" : (error_code.instruction_fetch ? "execute" : (error_code.reserved_write ? "write reserved metadata" : "read")), error_code.readable ? "present" : "non-present", faulting_address);

#ifdef __x86_64__
    dbgprintf("RAX=%016lx  RBX=%016lx  RCX=%016lx  RDX=%016lx\nRSI=%016lx  RDI=%016lx  RSP=%016lx  RBP=%016lx\nR8 =%016lx  R9 =%016lx  R10=%016lx  R11=%016lx\nR12=%016lx  R13=%016lx  R14=%016lx  R15=%016lx\nRIP=%016lx  RFL=%016lx", regs->RAX, regs->RBX, regs->RCX, regs->RDX, regs->RSI, regs->RDI, regs->RSP, regs->RBP, regs->R8, regs->R9, regs->R10, regs->R11, regs->R12, regs->R13, regs->R14, regs->R15, regs->RIP, regs->RFLAGS);
    dbgprintf("\nCS=%04hx  DS=%04hx\n", regs->CS, regs->DS);
    dbgprintf("CR3=%016lx\n", regs->CR3);

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
#endif

    dbgputs("\nThreads:\n");

    Scheduling::Scheduler::PrintThreads(stddebug);

    // Output all to stdout after in case framebuffer writes cause a page fault

    if (VGADevice == nullptr)
        dbgprintf("\nWARNING VGA Device unavailable.\n");
    else {
        VGADevice->ClearScreen(background);
        VGADevice->SetBackgroundColour(background);
        VGADevice->SetCursorPosition({0,0});
        VGADevice->SwapBuffers(false);

        g_CurrentTTY->SetVGADevice(VGADevice);

        puts("KERNEL PANIC!\n");
        printf("Page fault in %s-mode at %lp while trying to %s a %s page at address %lp\n", error_code.user ? "user" : "kernel", current_address, error_code.writable ? "write" : (error_code.instruction_fetch ? "execute" : (error_code.reserved_write ? "write reserved metadata" : "read")), error_code.readable ? "present" : "non-present", faulting_address);

#ifdef __x86_64__
        printf("RAX=%016lx  RBX=%016lx  RCX=%016lx  RDX=%016lx\nRSI=%016lx  RDI=%016lx  RSP=%016lx  RBP=%016lx\nR8 =%016lx  R9 =%016lx  R10=%016lx  R11=%016lx\nR12=%016lx  R13=%016lx  R14=%016lx  R15=%016lx\nRIP=%016lx  RFL=%016lx", regs->RAX, regs->RBX, regs->RCX, regs->RDX, regs->RSI, regs->RDI, regs->RSP, regs->RBP, regs->R8, regs->R9, regs->R10, regs->R11, regs->R12, regs->R13, regs->R14, regs->R15, regs->RIP, regs->RFLAGS);
        printf("\nCS=%04hx  DS=%04hx\n", regs->CS, regs->DS);
        printf("CR3=%016lx\n", regs->CR3);
#endif
    }

    while (true) {

    }
}