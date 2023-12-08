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

#include "Scheduler.hpp"

#include <assert.h>
#include <stdio.h>

#include <process.h>

#include <HAL/hal.hpp>

#ifdef __x86_64__
#include <arch/x86_64/Scheduling/taskutil.hpp>
#include <arch/x86_64/Stack.hpp>
#include <arch/x86_64/Memory/PagingUtil.hpp>
#include <arch/x86_64/io.h>
#include <arch/x86_64/interrupts/pic.hpp>
#endif

namespace Scheduling {

    size_t g_ticks = 0;
    
    namespace Scheduler {

        LinkedList::SimpleLinkedList<Process> g_processes;
        LinkedList::SimpleLinkedList<Thread> g_kernel_threads;
        LinkedList::SimpleLinkedList<Thread> g_high_threads;
        LinkedList::SimpleLinkedList<Thread> g_normal_threads;
        LinkedList::SimpleLinkedList<Thread> g_low_threads;
        LinkedList::SimpleLinkedList<Thread> g_sleeping_threads;
        uint8_t g_kernel_run_count = 0; // the amount of times a kernel thread has been run in a row
        uint8_t g_high_run_count = 0; // the amount of times a high thread has been run in a row
        uint8_t g_normal_run_count = 0; // the amount of times a normal thread has been run in a row
        uint64_t g_total_threads = 0;
        bool g_running = false;
        Thread* g_current = nullptr;
        pid_t g_NextPID = 0;

        void AddProcess(Process* process) {
            g_processes.insert(process);
            process->SetPID(g_NextPID);
            g_NextPID++;
        }

        void RemoveProcess(Process* process) {
            g_processes.remove(process);
        }

        void ScheduleThread(Thread* thread) {
            assert(thread != nullptr);
            Priority thread_priority = thread->GetParent()->GetPriority();
            if (g_processes.getIndex(thread->GetParent()) == UINT64_MAX)
                AddProcess(thread->GetParent());
            CPU_Registers* regs = thread->GetCPURegisters();
            fast_memset(regs, 0, DIV_ROUNDUP(sizeof(CPU_Registers), 8));
#ifdef __x86_64__
            if ((thread->GetFlags() & CREATE_STACK)) {
                x86_64_GetNewStack(thread->GetParent()->GetPageManager(), regs, KiB(64));
                thread->GetParent()->SyncRegion();
            }
            else
                regs->RSP = (uint64_t)x86_64_get_stack_ptr();
            thread->SetStack(regs->RSP);
            regs->RIP = (uint64_t)thread->GetEntry();
            regs->RFLAGS = (1 << 9) | (1 << 1); // IF Flag and Reserved (always 1)
            regs->CR3 = (uint64_t)(thread->GetParent()->GetPageManager()->GetPageTable().GetRootTablePhysical()) & 0x000FFFFFFFFFF000;
            regs->RDI = (uint64_t)thread->GetEntryData();
#endif
            if (thread_priority == Priority::KERNEL) {
#ifdef __x86_64__
                regs->RSP -= 8;
                *(uint64_t*)(regs->RSP) = (uint64_t)(void*)&Scheduling::Scheduler::End;
                regs->RSP -= 8;
                *(uint64_t*)(regs->RSP) = (uint64_t)(void*)&x86_64_kernel_thread_end;
                regs->CS = 0x08; // Kernel Code Segment
                regs->DS = 0x10; // Kernel Data Segment
#else
#error Unkown Architecture
#endif
                g_kernel_threads.insert(thread);
            }
            else {
#ifdef __x86_64__
                regs->CS = 0x23; // User Code Segment
                regs->DS = 0x1b; // User Data Segment
#endif
                switch (thread_priority) {
                    case Priority::HIGH:
                        g_high_threads.insert(thread);
                        break;
                    case Priority::NORMAL:
                        g_normal_threads.insert(thread);
                        break;
                    case Priority::LOW:
                        g_low_threads.insert(thread);
                        break;
                    default:
                        assert(false);
                        return; // just in case assertions are disabled
                }
            }
            g_total_threads++;
        }

        void RemoveThread(Thread* thread) {
            if (thread == nullptr)
                return;
            bool success = false;
            switch (thread->GetParent()->GetPriority()) {
                case Priority::KERNEL: {
                    uint64_t i = g_kernel_threads.getIndex(thread);
                    if (i != UINT64_MAX) {
                        g_kernel_threads.remove(i);
                        success = true;
                    }
                    break;
                }
                case Priority::HIGH: {
                    uint64_t i = g_high_threads.getIndex(thread);
                    if (i != UINT64_MAX) {
                        g_high_threads.remove(i);
                        success = true;
                    }
                    break;
                }
                case Priority::NORMAL: {
                    uint64_t i = g_normal_threads.getIndex(thread);
                    if (i != UINT64_MAX) {
                        g_normal_threads.remove(i);
                        success = true;
                    }
                    break;
                }
                case Priority::LOW: {
                    uint64_t i = g_low_threads.getIndex(thread);
                    if (i != UINT64_MAX) {
                        g_low_threads.remove(i);
                        success = true;
                    }
                    break;
                }
                default:
                    return;
            }
            if (!success) {
                if (thread == g_current) {
                    g_current = nullptr;
                    success = true;
                    g_total_threads--;
                    PickNext();
                }
            }
            else
                g_total_threads--;
        }

        void __attribute__((noreturn)) Start() {
#ifdef __x86_64__
            x86_64_DisableInterrupts(); // task switch code re-enables them
#endif
            PickNext();
            if (g_current == nullptr) {
                PANIC("Scheduler: No available threads. This means all threads have ended and there is nothing else to run.");
            }
            assert(g_current->GetCPURegisters() != nullptr);
            g_ticks = 0;
            g_running = true;
#ifdef __x86_64__
            x86_64_set_kernel_gs_base((uint64_t)(g_current->GetStackRegisterFrame()));
            if (g_current->GetParent()->GetPriority() == Priority::KERNEL)
                x86_64_kernel_switch(g_current->GetCPURegisters());
            else
                x86_64_enter_user(g_current->GetCPURegisters());
#endif
            PANIC("Failed to start Scheduler. This should never happen and most likely means the task switch code for the relevant architecture returned.");
        }

        void __attribute__((noreturn)) Next() {
            if (g_current == nullptr) {
                PANIC("Scheduler: No available threads. This means all threads have ended and there is nothing else to run.");
            }
            assert(g_current->GetCPURegisters() != nullptr);
            g_ticks = 0;
#ifdef __x86_64__
            x86_64_set_kernel_gs_base((uint64_t)(g_current->GetStackRegisterFrame()));
            if (g_current->GetParent()->GetPriority() == Priority::KERNEL)
                x86_64_kernel_switch(g_current->GetCPURegisters());
            else
                x86_64_enter_user(g_current->GetCPURegisters());
#endif
            PANIC("Failed to switch to new thread. This should never happen and most likely means the task switch code for the relevant architecture returned.");
        }

        void Next(void* iregs) {
            if (g_current == nullptr) {
                PANIC("Scheduler: No available threads. This means all threads have ended and there is nothing else to run.");
            }
            assert(g_current->GetCPURegisters() != nullptr);
            g_ticks = 0;
#ifdef __x86_64__
            x86_64_set_kernel_gs_base((uint64_t)(g_current->GetStackRegisterFrame()));
            if (g_current->GetParent()->GetPriority() == Priority::KERNEL)
                x86_64_kernel_switch(g_current->GetCPURegisters());
            else {
                x86_64_PrepareNewRegisters((x86_64_Interrupt_Registers*)iregs, g_current->GetCPURegisters());
                return;
            }
#endif
            PANIC("Failed to switch to new thread. This should never happen and most likely means the task switch code for the relevant architecture returned.");
        }

        void __attribute__((noreturn)) Next(Thread* thread) {
#ifdef __x86_64__
            x86_64_DisableInterrupts();
#endif
            assert(thread != nullptr);
            assert(thread->GetCPURegisters() != nullptr);
            g_ticks = 0;
#ifdef __x86_64__
            x86_64_set_kernel_gs_base((uint64_t)(thread->GetStackRegisterFrame()));
            if (thread->GetParent()->GetPriority() == Priority::KERNEL)
                x86_64_kernel_switch(thread->GetCPURegisters());
            else
                x86_64_enter_user(thread->GetCPURegisters());
#endif
            PANIC("Failed to switch to new thread. This should never happen and most likely means the task switch code for the relevant architecture returned.");
        }

        void End() {
            // TODO: call any destructors or other destruction function
            if (g_current->GetFlags() & CREATE_STACK)
                g_current->GetParent()->GetPageManager()->FreePages((void*)(g_current->GetStack() - KiB(64)));
            g_current = nullptr;
            PickNext();
            Next();
        }

        Thread* GetCurrent() {
            return g_current;
        }

        void PickNext() {
            if (g_total_threads == 0) {
                PANIC("Scheduler: No available threads. This means all threads have ended and there is nothing else to run.");
            }
            if (g_total_threads == 1 && g_current != nullptr)
                return;
            if (g_current != nullptr) {
                switch (g_current->GetParent()->GetPriority()) {
                    case Priority::KERNEL:
                        g_kernel_threads.insert(g_current);
                        g_kernel_run_count++;
                        break;
                    case Priority::HIGH:
                        g_high_threads.insert(g_current);
                        g_high_run_count++;
                        break;
                    case Priority::NORMAL:
                        g_normal_threads.insert(g_current);
                        g_normal_run_count++;
                        break;
                    case Priority::LOW:
                        g_low_threads.insert(g_current);
                        break;
                    default:
                        PANIC("Scheduler: A thread has run with an unknown priority.");
                        return; // unnecessary, but only here to remove compiler warnings
                }
            }
            if ((g_kernel_run_count >= 4 || g_kernel_threads.getCount() == 0) && (g_high_threads.getCount() > 0 || g_normal_threads.getCount() > 0 || g_low_threads.getCount() > 0)) { // time to select a high thread
                if ((g_high_run_count >= 4 || g_high_threads.getCount() == 0) && (g_normal_threads.getCount() > 0 || g_low_threads.getCount() > 0)) { // time to select a normal thread
                    if ((g_normal_run_count >= 4 || g_normal_threads.getCount() == 0) && g_low_threads.getCount() > 0) { // time to select a low thread
                        g_current = g_low_threads.get(0);
                        g_low_threads.remove(UINT64_C(0));
                        g_normal_run_count = 0;
                    }
                    else {
                        g_current = g_normal_threads.get(0);
                        g_normal_threads.remove(UINT64_C(0));
                    }
                    g_high_run_count = 0;
                }
                else {
                    g_current = g_high_threads.get(0);
                    g_high_threads.remove(UINT64_C(0));
                }
                g_kernel_run_count = 0;
            }
            else {
                if (g_kernel_threads.getCount() == 0) {
                    g_current = nullptr;
                }
                else {
                    g_current = g_kernel_threads.get(0);
                    g_kernel_threads.remove(UINT64_C(0));
                }
            }
        }


        void TimerTick(void* iregs) {
            g_ticks++;
            for (uint64_t i = 0; i < g_sleeping_threads.getCount(); i++) {
                Thread* thread = g_sleeping_threads.get(i);
                thread->SetRemainingSleepTime(thread->GetRemainingSleepTime() - MS_PER_TICK);
                if (thread->GetRemainingSleepTime() == 0) {
                    thread->SetSleeping(false);
                    g_sleeping_threads.remove(i);
                    ReaddThread(thread);
                    i--;
                }
            }
            if (g_ticks == TICKS_PER_SCHEDULER_CYCLE) {
                g_ticks = 0;
                if (!g_running)
                    return; // Nothing to switch to
                if (g_current == nullptr) {
                    PANIC("Scheduler: No available threads. This means all threads have ended and there is nothing else to run.");
                }
                PickNext();
#ifdef __x86_64__
                if (g_current->GetParent()->GetPriority() == Priority::KERNEL)
                    x86_64_PIC_sendEOI(0);
#endif
                Next(iregs);
            }
        }

        bool isRunning() {
            return g_running;
        }

        void Stop() {
            g_running = false;
        }

        void Resume() {
            g_running = true;
            if (g_current == nullptr)
                PickNext();
            if (g_current == nullptr) {
                PANIC("Scheduler: No available threads. This means all threads have ended and there is nothing else to run.");
            }
            Next();
        }

        void SleepThread(Thread* thread, uint64_t ms) {
            assert(thread != nullptr);
            // Remove the thread
            bool found = false;
            switch (thread->GetParent()->GetPriority()) {
            case Priority::KERNEL: {
                uint64_t i = g_kernel_threads.getIndex(thread);
                if (i != UINT64_MAX) {
                    g_kernel_threads.remove(i);
                    found = true;
                }
                break;
            }
            case Priority::HIGH: {
                uint64_t i = g_high_threads.getIndex(thread);
                if (i != UINT64_MAX) {
                    g_high_threads.remove(i);
                    found = true;
                }
                break;
            }
            case Priority::NORMAL: {
                uint64_t i = g_normal_threads.getIndex(thread);
                if (i != UINT64_MAX) {
                    g_normal_threads.remove(i);
                    found = true;
                }
                break;
            }
            case Priority::LOW: {
                uint64_t i = g_low_threads.getIndex(thread);
                if (i != UINT64_MAX) {
                    g_low_threads.remove(i);
                    found = true;
                }
                break;
            }
            default:
                return;
            }
            if (!found) {
                if (thread == g_current) {
                    g_current = nullptr;
                    found = true;
                    g_total_threads--;
                    PickNext();
                }
            }
            else
                return;

            thread->SetSleeping(true);
            thread->SetRemainingSleepTime(ALIGN_UP(ms, MS_PER_TICK));
            assert(thread->GetCPURegisters() != nullptr);
            //thread->GetCPURegisters()->RIP = (uint64_t)return_address;
            g_sleeping_threads.insert(thread);
            Next();
        }

        void ReaddThread(Thread* thread) {
            assert(thread != nullptr);
            switch (thread->GetParent()->GetPriority()) {
            case Priority::KERNEL:
                g_kernel_threads.insert(thread);
                break;
            case Priority::HIGH:
                g_high_threads.insert(thread);
                break;
            case Priority::NORMAL:
                g_normal_threads.insert(thread);
                break;
            case Priority::LOW:
                g_low_threads.insert(thread);
                break;
            default:
                return;
            }
        }

        void PrintThreads(fd_t file) {
            if (g_kernel_threads.getCount() > 0) {
                fprintf(file, "Kernel Threads:\n");
                for (uint64_t i = 0; i < g_kernel_threads.getCount(); i++) {
                    g_kernel_threads.get(i)->PrintInfo(file);
                    fputc(file, '\n');
                }
            }
            if (g_high_threads.getCount() > 0) {
                fprintf(file, "High Threads:\n");
                for (uint64_t i = 0; i < g_high_threads.getCount(); i++) {
                    g_high_threads.get(i)->PrintInfo(file);
                    fputc(file, '\n');
                }
            }
            if (g_normal_threads.getCount() > 0) {
                fprintf(file, "Normal Threads:\n");
                for (uint64_t i = 0; i < g_normal_threads.getCount(); i++) {
                    g_normal_threads.get(i)->PrintInfo(file);
                    fputc(file, '\n');
                }
            }
            if (g_low_threads.getCount() > 0) {
                fprintf(file, "Low Threads:\n");
                for (uint64_t i = 0; i < g_low_threads.getCount(); i++) {
                    g_low_threads.get(i)->PrintInfo(file);
                    fputc(file, '\n');
                }
            }
            if (g_sleeping_threads.getCount() > 0) {
                fprintf(file, "Sleeping Threads:\n");
                for (uint64_t i = 0; i < g_sleeping_threads.getCount(); i++) {
                    g_sleeping_threads.get(i)->PrintInfo(file);
                    fputc(file, '\n');
                }
            }
            if (g_current != nullptr) {
                fprintf(file, "Current Thread:\n");
                g_current->PrintInfo(file);
                fputc(file, '\n');
            }
        }
    }
}
