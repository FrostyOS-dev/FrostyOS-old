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

void __attribute__((noreturn)) PageFaultHandler(PageFaultErrorCode error_code, void* faulting_address, void* current_address, CPU_Registers* regs) {
    Scheduling::Thread* thread = Scheduling::Scheduler::GetCurrent();
    Scheduling::Process* process = nullptr;
    if (thread != nullptr) {
        fast_memcpy(thread->GetCPURegisters(), regs, sizeof(CPU_Registers)); // save the registers
        process = thread->GetParent();
    }
    if (error_code.user && Scheduling::Scheduler::isRunning() && process != nullptr)
        process->ReceiveSignal(SIGSEGV);
    
    Scheduling::Scheduler::Stop();

    dbgprintf("Page fault in %s-mode at %lp while trying to %s a %s page at address %lp\n", error_code.user ? "user" : "kernel", current_address, error_code.writable ? "write" : (error_code.instruction_fetch ? "execute" : (error_code.reserved_write ? "write reserved metadata" : "read")), error_code.readable ? "present" : "non-present", faulting_address);
    
#ifdef __x86_64__
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

    printf("Page fault in %s-mode at %lp while trying to %s a %s page at address %lp\n", error_code.user ? "user" : "kernel", current_address, error_code.writable ? "write" : (error_code.instruction_fetch ? "execute" : (error_code.reserved_write ? "write reserved metadata" : "read")), error_code.readable ? "present" : "non-present", faulting_address);

    while (true) {

    }
}