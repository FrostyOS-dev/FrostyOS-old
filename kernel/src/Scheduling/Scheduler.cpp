#include "Scheduler.hpp"

#include <assert.h>
#include <stdio.hpp>

#include <HAL/hal.hpp>

#ifdef __x86_64__
#include <arch/x86_64/Scheduling/taskutil.hpp>
#include <arch/x86_64/Stack.h>
#include <arch/x86_64/Memory/PagingUtil.hpp>
#include <arch/x86_64/io.h>
#endif

namespace Scheduling {

    size_t g_ticks = 0;
    
    namespace Scheduler {

        LinkedList::SimpleLinkedList<Process> g_processes;
        LinkedList::SimpleLinkedList<Thread> g_kernel_threads;
        bool g_running = false;
        Thread* g_current = nullptr;

        void AddProcess(Process* process) {
            assert(process->GetPriority() == Priority::KERNEL); // only kernel priority is supported
            g_processes.insert(process);
        }

        void ScheduleThread(Thread* thread) {
            fprintf(VFS_DEBUG, "[%s] INFO: thread=%lp\n", __extension__ __PRETTY_FUNCTION__, thread);
            assert(thread != nullptr);
            assert(thread->GetParent()->GetPriority() == Priority::KERNEL); // only kernel priority is supported
            if (g_processes.getIndex(thread->GetParent()) == UINT64_MAX)
                AddProcess(thread->GetParent());
            CPU_Registers* regs = thread->GetCPURegisters();
            fast_memset(regs, 0, DIV_ROUNDUP(sizeof(CPU_Registers), 8));
#ifdef __x86_64__
            if ((thread->GetFlags() & CREATE_STACK))
                x86_64_GetNewStack(thread->GetParent()->GetPageManager(), regs, KiB(64));
            else
                regs->RSP = (uint64_t)x86_64_get_stack_ptr();
            thread->SetStack(regs->RSP);
            regs->RSP -= 8;
            *(uint64_t*)(regs->RSP) = (uint64_t)(void*)&x86_64_kernel_thread_end;
            regs->RSP -= 8;
            *(uint64_t*)(regs->RSP) = (uint64_t)(void*)&Scheduling::Scheduler::End;
            regs->RIP = (uint64_t)thread->GetEntry();
            regs->RDI = (uint64_t)thread->GetEntryData();
            regs->CR3 = x86_64_GetCR3();
            regs->CS = 0x08; // Kernel Code Segment
            regs->DS = 0x10; // Kernel Data Segment
            regs->RFLAGS = 1 << 9; // IF Flag
#else
#error Unkown Architecture
#endif
            g_kernel_threads.insert(thread);
        }

        void __attribute__((noreturn)) Start() {
#ifdef __x86_64__
            x86_64_DisableInterrupts(); // task switch code re-enables them
#endif
            g_current = g_kernel_threads.get(0);
            g_kernel_threads.remove(UINT64_C(0));
            assert(g_current != nullptr);
            assert(g_current->GetCPURegisters() != nullptr);
            g_ticks = 0;
            g_running = true;
#ifdef __x86_64__
            x86_64_kernel_switch(g_current->GetCPURegisters());
#endif
            WorldOS::Panic("Failed to start Scheduler. This should never happen and most likely means the task switch code for the relevant architecture returned.", nullptr, false);
        }

        void __attribute__((noreturn)) Next() {
            assert(g_current != nullptr);
            assert(g_current->GetCPURegisters() != nullptr);
            g_ticks = 0;
#ifdef __x86_64__
            x86_64_kernel_switch(g_current->GetCPURegisters());
#endif
            WorldOS::Panic("Failed to switch to new thread. This should never happen and most likely means the task switch code for the relevant architecture returned.", nullptr, false);
        }

        void __attribute__((noreturn)) Next(Thread* thread) {
#ifdef __x86_64__
            x86_64_DisableInterrupts();
#endif
            assert(thread != nullptr);
            assert(thread->GetCPURegisters() != nullptr);
            g_ticks = 0;
#ifdef __x86_64__
            x86_64_kernel_switch(thread->GetCPURegisters());
#endif
            WorldOS::Panic("Failed to switch to new thread. This should never happen and most likely means the task switch code for the relevant architecture returned.", nullptr, false);
        }

        CPU_Registers* Next(CPU_Registers* regs) {
            if (!g_running) {
                fprintf(VFS_DEBUG, "[Scheduler] WARN: Scheduler not running.\n");
                return nullptr;
            }
            if (g_kernel_threads.getCount() < 1) {
                fprintf(VFS_DEBUG, "[Scheduler] WARN: Nothing to switch to. %ld\n", g_kernel_threads.getCount());
                return nullptr; // Nothing to switch to
            }
            assert(g_processes.getCount() > 0);
            assert(g_kernel_threads.getCount() > 0); // temporary until more priority levels are implemented
            assert(regs != nullptr);
            fast_memcpy(g_current->GetCPURegisters(), regs, sizeof(CPU_Registers) / 8);
            g_kernel_threads.insert(g_current);
            g_current = g_kernel_threads.get(0);
            g_kernel_threads.remove(UINT64_C(0));
            fprintf(VFS_DEBUG, "Switching to %lp...\n", g_current->GetCPURegisters()->RIP);
            return g_current->GetCPURegisters();
        }

        void End() {
            // TODO: call any destructors or other destruction function
            if (g_current->GetFlags() & CREATE_STACK)
                g_current->GetParent()->GetPageManager()->FreePages((void*)(g_current->GetStack() - KiB(64)));
            g_current = g_kernel_threads.get(0);
            g_kernel_threads.remove(UINT64_C(0));
            Next();
        }


        CPU_Registers* TimerTick(CPU_Registers* regs) {
            g_ticks++;
            if (g_ticks == TICKS_PER_SCHEDULER_CYCLE) {
                regs = Next(regs);
                g_ticks = 0;
            }
            return regs;
        }
    }
}