#include "Scheduler.hpp"

#include <assert.h>
#include <stdio.hpp>

#include <HAL/hal.hpp>

#ifdef __x86_64__
#include <arch/x86_64/Scheduling/taskutil.hpp>
#include <arch/x86_64/Stack.h>
#include <arch/x86_64/Memory/PagingUtil.hpp>
#endif

namespace Scheduling {

    size_t g_ticks = 0;

    Scheduler* g_Scheduler = nullptr;

    Scheduler::Scheduler() : m_running(false) {

    }
    
    Scheduler::~Scheduler() {

    }

    void Scheduler::AddProcess(Process* process) {
        assert(process->GetPriority() == Priority::KERNEL); // only kernel priority is supported
        m_processes.insert(process);
    }

    void Scheduler::ScheduleThread(Thread* thread) {
        assert(thread != nullptr);
        assert(thread->GetParent()->GetPriority() == Priority::KERNEL); // only kernel priority is supported
        if (m_processes.getIndex(thread->GetParent()) == UINT64_MAX)
            AddProcess(thread->GetParent());
        CPU_Registers* regs = new CPU_Registers;
        fast_memset(regs, 0, DIV_ROUNDUP(sizeof(CPU_Registers), 8));
#ifdef __x86_64__
        if ((thread->GetFlags() & CREATE_STACK))
            x86_64_GetNewStack(thread->GetParent()->GetPageManager(), regs, KiB(64));
        else
            regs->RSP = (uint64_t)x86_64_get_stack_ptr();
        regs->RIP = (uint64_t)thread->GetEntry();
        regs->RDI = (uint64_t)thread->GetEntryData();
        regs->CR3 = x86_64_GetCR3();
        regs->CS = 0x08; // Kernel Code Segment
        regs->DS = 0x10; // Kernel Data Segment
        regs->RFLAGS = 1 << 9; // IF Flag
#else
#error Unkown Architecture
#endif
        thread->UpdateCPURegisters(regs);
        m_kernel_threads.insert(thread);
    }

    void __attribute__((noreturn)) Scheduler::Start() {
        Thread** thread_ptr = m_kernel_threads.getHead();
        assert(thread_ptr != nullptr);
        Thread* thread = *thread_ptr;
        assert(thread != nullptr);
        assert(thread->GetCPURegisters() != nullptr);
        m_running = true;
#ifdef __x86_64__
        x86_64_kernel_switch(thread->GetCPURegisters());
#endif
        WorldOS::Panic("Failed to start Scheduler. This should never happen and most likely means the task switch code for the relevant architecture returned.", nullptr, false);
    }

    CPU_Registers* Scheduler::Next(CPU_Registers* regs) {
        if (!m_running) {
            fprintf(VFS_DEBUG, "[Scheduler] WARN: Scheduler not running.\n");
            return nullptr;
        }
        if (m_kernel_threads.getCount() < 2) {
            fprintf(VFS_DEBUG, "[Scheduler] WARN: Nothing to switch to. %ld\n", m_kernel_threads.getCount());
            return nullptr; // Nothing to switch to
        }
        assert(m_processes.getCount() > 0);
        assert(m_kernel_threads.getCount() > 0); // temporary until more priority levels are implemented
        Thread** first_ptr = m_kernel_threads.getHead();
        assert(first_ptr != nullptr);
        Thread* first = *first_ptr;
        //fprintf(VFS_DEBUG, "%lp:%lp ", first_ptr, first);
        assert(regs != nullptr);
        fast_memcpy(first->GetCPURegisters(), regs, sizeof(CPU_Registers) / 8);
        m_kernel_threads.rotateLeft();
        first_ptr = m_kernel_threads.getHead();
        assert(first_ptr != nullptr);
        first = *first_ptr;
        //fprintf(VFS_DEBUG, "%lp:%lp\n", first_ptr, first);
        fprintf(VFS_DEBUG, "Switching to %lp...\n", first->GetCPURegisters()->RIP);
        return first->GetCPURegisters();
    }

    CPU_Registers* TimerTick(CPU_Registers* regs) {
        g_ticks++;
        if (g_ticks == TICKS_PER_SCHEDULER_CYCLE) {
            if (g_Scheduler != nullptr)
                regs = g_Scheduler->Next(regs);
            else
                fprintf(VFS_DEBUG, "Switch failed.\n");
            g_ticks = 0;
        }
        return regs;
    }
}