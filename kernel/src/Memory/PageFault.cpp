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
#include <util.h>

#include <Scheduling/Scheduler.hpp>
#include <Scheduling/Process.hpp>

#ifdef __x86_64__
#include <arch/x86_64/ELFSymbols.hpp>
#include <arch/x86_64/Stack.hpp>
#include <arch/x86_64/Processor.hpp>
#endif

void __attribute__((noreturn)) PageFaultHandler(PageFaultErrorCode error_code, void* faulting_address, void* current_address, CPU_Registers* regs) {
    Scheduling::Thread* thread = Scheduling::Scheduler::GetCurrent();
    Scheduling::Process* process = nullptr;
    if (thread != nullptr) {
        memcpy(thread->GetCPURegisters(), regs, sizeof(CPU_Registers)); // save the registers
        process = thread->GetParent();
    }
    if (Scheduling::Scheduler::isRunning() && process != nullptr) {
        if (error_code.user) { // fairly straightforward, the error is in user mode, so we just send SIGSEGV
            process->ReceiveSignal(SIGSEGV);
#ifdef __x86_64__
            x86_64_Panic("Sending SIGSEGV to process failed after user-mode page fault.", regs, false);
#endif
        }
        else { // A bit more complicated as we are in kernel mode.
            // First we check if this occurred in the current threads kernel stack guard pages
            void* kernelStack = (void*)(thread->GetKernelStack() - KERNEL_STACK_SIZE);
            void* preStackGuardStart = (void*)((uint64_t)kernelStack - PAGE_SIZE);
            void* postStackGuardStart = (void*)((uint64_t)kernelStack + KERNEL_STACK_SIZE);
            dbgprintf("Potential kernel stack violation! kernelStack = %lp, preStackGuardStart = %lp, postStackGuardStart = %lp\n", kernelStack, preStackGuardStart, postStackGuardStart);
            if ((faulting_address >= preStackGuardStart && faulting_address < kernelStack) || (faulting_address >= postStackGuardStart && faulting_address < (void*)((uint64_t)postStackGuardStart + PAGE_SIZE))) {
                // We are in a kernel stack guard page, so we PANIC with information about the stack.
                uint64_t bytesOutsideStack = 0;
                if (faulting_address >= preStackGuardStart && faulting_address < kernelStack)
                    bytesOutsideStack = (uint64_t)kernelStack - (uint64_t)faulting_address;
                else if (faulting_address >= postStackGuardStart && faulting_address < (void*)((uint64_t)postStackGuardStart + PAGE_SIZE))
                    bytesOutsideStack = (uint64_t)faulting_address - (uint64_t)postStackGuardStart;
                char buffer[256]; // should never need a buffer this big, but just in case...
                // since the stack grows down on x86, an overflow is actually when the stack goes into the pre-stack guard page
                snprintf(buffer, 256, "Kernel stack %s of %lu bytes in %s process at %lp", faulting_address >= postStackGuardStart ? "underflow" : "overflow", bytesOutsideStack, process->GetPriority() == Scheduling::Priority::KERNEL ? "kernel" : "user", current_address);
#ifdef __x86_64__
                x86_64_Panic(buffer, regs, false);
#endif
            }
        }
    }
    else {
        // We must be in early stages of kernel start-up, but we can still check if we are in a kernel stack guard page.
        Processor* processor = GetCurrentProcessor();
        if (processor != nullptr) { // if its nullptr, then it shouldn't be possible that this is a stack issue.
            // Check the kernel stack specified in the processor
            void* kernelStack = (void*)processor->GetKernelStack();
            void* preStackGuardStart = (void*)((uint64_t)kernelStack - PAGE_SIZE);
            void* postStackGuardStart = (void*)((uint64_t)kernelStack + KERNEL_STACK_SIZE);
            if ((faulting_address >= preStackGuardStart && faulting_address < kernelStack) || (faulting_address >= postStackGuardStart && faulting_address < (void*)((uint64_t)postStackGuardStart + PAGE_SIZE))) {
                // We are in a kernel stack guard page, so we PANIC with information about the stack.
                uint64_t bytesOutsideStack = 0;
                if (faulting_address >= preStackGuardStart && faulting_address < kernelStack)
                    bytesOutsideStack = (uint64_t)kernelStack - (uint64_t)faulting_address;
                else if (faulting_address >= postStackGuardStart && faulting_address < (void*)((uint64_t)postStackGuardStart + PAGE_SIZE))
                    bytesOutsideStack = (uint64_t)faulting_address - (uint64_t)postStackGuardStart;
                char buffer[256]; // should never need a buffer this big, but just in case...
                // since the stack grows down on x86, an overflow is actually when the stack goes into the pre-stack guard page
                snprintf(buffer, 256, "Kernel stack %s of %lu bytes in kernel process at %lp", faulting_address >= postStackGuardStart ? "underflow" : "overflow", bytesOutsideStack, current_address);
#ifdef __x86_64__
                x86_64_Panic(buffer, regs, false);
#endif
            }
        }
    }

    char buffer[256]; // should never need a buffer this big, but just in case...
    snprintf(buffer, 256, "Page fault in %s-mode at %lp while trying to %s a %s page at address %lp", error_code.user ? "user" : "kernel", current_address, error_code.writable ? "write" : (error_code.instruction_fetch ? "execute" : (error_code.reserved_write ? "write reserved metadata" : "read")), error_code.readable ? "present" : "non-present", faulting_address);

#ifdef __x86_64__
    x86_64_Panic(buffer, regs, false);
#endif

    while (true) {

    }
}