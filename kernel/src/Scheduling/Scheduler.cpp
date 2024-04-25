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

#include "Scheduler.hpp"

#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <spinlock.h>

#include <process.h>

#include <HAL/hal.hpp>

#ifdef __x86_64__
#include <arch/x86_64/io.h>
#include <arch/x86_64/Processor.hpp>
#include <arch/x86_64/Stack.hpp>

#include <arch/x86_64/interrupts/pic.hpp>

#include <arch/x86_64/interrupts/APIC/IOAPIC.hpp>

#include <arch/x86_64/Memory/PagingUtil.hpp>

#include <arch/x86_64/Scheduling/taskutil.hpp>
#endif

namespace Scheduling {
    
    namespace Scheduler {

        ProcessorInfo g_BSPInfo;

        LinkedList::LockableLinkedList<ProcessorInfo> g_processors;
        LinkedList::LockableLinkedList<Process> g_processes;
        LinkedList::LockableLinkedList<Thread> g_kernel_threads;
        LinkedList::LockableLinkedList<Thread> g_high_threads;
        LinkedList::LockableLinkedList<Thread> g_normal_threads;
        LinkedList::LockableLinkedList<Thread> g_low_threads;
        LinkedList::LockableLinkedList<Thread> g_sleeping_threads;
        LinkedList::LockableLinkedList<Thread> g_blocked_threads;
        uint64_t g_total_threads = 0;
        pid_t g_NextPID = 0;
        bool g_scheduler_running = false;
        spinlock_new(g_global_lock);

        void ClearGlobalData() {
            g_processors.unlock();
            g_processes.unlock();
            g_kernel_threads.unlock();
            g_high_threads.unlock();
            g_normal_threads.unlock();
            g_low_threads.unlock();
            g_sleeping_threads.unlock();
            g_blocked_threads.unlock();
            g_total_threads = 0;
            g_NextPID = 0;
            g_scheduler_running = false;
            spinlock_init(&g_global_lock);
        }

        void InitBSPInfo() {
            g_BSPInfo.processor = &g_BSP;
            g_BSPInfo.id = 0;
            g_BSPInfo.kernel_run_count = 0;
            g_BSPInfo.high_run_count = 0;
            g_BSPInfo.normal_run_count = 0;
            g_BSPInfo.current_thread = nullptr;
            g_BSPInfo.running = false;
            g_BSPInfo.ticks = 0;
            g_BSPInfo.start_allowed = 0;
            g_processors.insert(&g_BSPInfo); // no point in locking, as we are the only ones running
#ifdef __x86_64__
            x86_64_set_kernel_gs_base((uint64_t)&g_BSPInfo);
#endif
        }

        void AddProcess(Process* process) {
            g_processes.lock();
            g_processes.insert(process);
            spinlock_acquire(&g_global_lock);
            process->SetPID(g_NextPID);
            g_NextPID++;
            spinlock_release(&g_global_lock);
            g_processes.unlock();
        }

        void RemoveProcess(Process* process) {
            g_processes.lock();
            g_processes.remove(process);
            g_processes.unlock();
        }

        void ScheduleThread(Thread* thread) {
            assert(thread != nullptr);
            g_processes.lock(); // must be locked while accessing key information, so we know it is correct
            Priority thread_priority = thread->GetParent()->GetPriority();
            if (g_processes.getIndex(thread->GetParent()) == UINT64_MAX)
                AddProcess(thread->GetParent());
            g_processes.unlock();
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
                g_kernel_threads.lock();
                g_kernel_threads.insert(thread);
                g_kernel_threads.unlock();
            }
            else {
#ifdef __x86_64__
                regs->CS = 0x23; // User Code Segment
                regs->DS = 0x1b; // User Data Segment
#endif
                switch (thread_priority) {
                    case Priority::HIGH:
                        g_high_threads.lock();
                        g_high_threads.insert(thread);
                        g_high_threads.unlock();
                        break;
                    case Priority::NORMAL:
                        g_normal_threads.lock();
                        g_normal_threads.insert(thread);
                        g_normal_threads.unlock();
                        break;
                    case Priority::LOW:
                        g_low_threads.lock();
                        g_low_threads.insert(thread);
                        g_low_threads.unlock();
                        break;
                    default:
                        assert(false);
                        return; // just in case assertions are disabled
                }
            }
            spinlock_acquire(&g_global_lock);
            g_total_threads++;
            spinlock_release(&g_global_lock);
        }

        void RemoveThread(Thread* thread) {
            if (thread == nullptr)
                return;
            bool success = false;
            switch (thread->GetParent()->GetPriority()) {
                case Priority::KERNEL: {
                    g_kernel_threads.lock();
                    uint64_t i = g_kernel_threads.getIndex(thread);
                    if (i != UINT64_MAX) {
                        g_kernel_threads.remove(i);
                        success = true;
                    }
                    g_kernel_threads.unlock();
                    break;
                }
                case Priority::HIGH: {
                    g_high_threads.lock();
                    uint64_t i = g_high_threads.getIndex(thread);
                    if (i != UINT64_MAX) {
                        g_high_threads.remove(i);
                        success = true;
                    }
                    g_high_threads.unlock();
                    break;
                }
                case Priority::NORMAL: {
                    g_normal_threads.lock();
                    uint64_t i = g_normal_threads.getIndex(thread);
                    if (i != UINT64_MAX) {
                        g_normal_threads.remove(i);
                        success = true;
                    }
                    g_normal_threads.unlock();
                    break;
                }
                case Priority::LOW: {
                    g_low_threads.lock();
                    uint64_t i = g_low_threads.getIndex(thread);
                    if (i != UINT64_MAX) {
                        g_low_threads.remove(i);
                        success = true;
                    }
                    g_low_threads.unlock();
                    break;
                }
                default:
                    return;
            }
            if (!success) {
                g_processors.lock();
                for (uint64_t i = 0; i < g_processors.getCount(); i++) {
                    ProcessorInfo* proc = g_processors.get(i);
                    if (proc->current_thread == thread) {
                        proc->current_thread = nullptr;
                        success = true;
                        spinlock_acquire(&g_global_lock);
                        g_total_threads--;
                        spinlock_release(&g_global_lock);
                        PickNext();
                        break;
                    }
                }
                g_processors.unlock();
            }
            else
                g_total_threads--;
        }

        void AddProcessor(Processor* processor) {
            ProcessorInfo* info = new ProcessorInfo();
            info->processor = processor;
            info->id = g_processors.getCount();
            info->kernel_run_count = 0;
            info->high_run_count = 0;
            info->normal_run_count = 0;
            info->current_thread = nullptr;
            info->running = false;
            info->ticks = 0;
            info->start_allowed = 0;
            g_processors.lock();
            g_processors.insert(info);
            g_processors.unlock();
#ifdef __x86_64__
            x86_64_set_kernel_gs_base((uint64_t)info);
            x86_64_LocalAPIC* LAPIC = processor->GetLocalAPIC();
            if (LAPIC == nullptr)
                return;
#endif
            while (info->start_allowed == 0) {
                // wait for the processor to be allowed to start
#ifdef __x86_64__
                __asm__ volatile("pause" ::: "memory");
#endif
            }
            if (info->current_thread == nullptr) {
                PANIC("Scheduler: No available threads. This means all threads have ended and there is nothing else to run.");
            }
            assert(info->current_thread->GetCPURegisters() != nullptr);
#ifdef __x86_64__
            LAPIC->InitTimer();
#endif
            SetThreadFrame(info, info->current_thread->GetStackRegisterFrame());
#ifdef __x86_64__
            if (info->current_thread->GetParent()->GetPriority() == Priority::KERNEL) // NOTE: we do not lock the parent process here, as we are the only ones with access to it at this stage.
                x86_64_kernel_switch(info->current_thread->GetCPURegisters());
            else
                x86_64_enter_user(info->current_thread->GetCPURegisters());
#endif
            PANIC("Failed to start Scheduler. This should never happen and most likely means the task switch code for the relevant architecture returned.");
        }

        Processor* GetProcessor(uint8_t ID) {
            g_processors.lock();
            for (uint64_t i = 0; i < g_processors.getCount(); i++) {
                ProcessorInfo* info = g_processors.get(i);
#ifdef __x86_64__
                x86_64_LocalAPIC* LAPIC = info->processor->GetLocalAPIC();
                if (LAPIC == nullptr)
                    continue;
                if (LAPIC->GetID() == ID) {
                    g_processors.unlock();
                    return info->processor;
                }
#endif
            }
            g_processors.unlock();
            return nullptr;
        }

        ProcessorInfo* GetProcessorInfo(Processor* processor) {
            g_processors.lock();
            for (uint64_t i = 0; i < g_processors.getCount(); i++) {
                ProcessorInfo* info = g_processors.get(i);
                if (info->processor == processor) {
                    g_processors.unlock();
                    return info;
                }
            }
            g_processors.unlock();
            return nullptr;
        }

        void RemoveProcessor(Processor* processor) {
            g_processors.lock();
            for (uint64_t i = 0; i < g_processors.getCount(); i++) {
                ProcessorInfo* info = g_processors.get(i);
                if (info->processor == processor) {
                    g_processors.remove(i);
                    g_processors.unlock();
                    return;
                }
            }
            g_processors.unlock();
        }

        void SetThreadFrame(ProcessorInfo* info, Thread::Register_Frame* frame) {
            if (info == nullptr)
                return;
            memcpy(&info->thread_metadata, frame, sizeof(Thread::Register_Frame));
        }

        void InitProcessorTimers() {
            g_processors.lock();
            for (uint64_t i = 0; i < g_processors.getCount(); i++) {
                ProcessorInfo* info = g_processors.get(i);
#ifdef __x86_64__
                x86_64_LocalAPIC* LAPIC = info->processor->GetLocalAPIC();
                if (LAPIC == nullptr)
                    continue;
                LAPIC->AllowInitTimer();
#endif
            }
            g_processors.unlock();
        }

        void __attribute__((noreturn)) Start() {
            g_processors.lock();
            for (uint64_t i = 0; i < g_processors.getCount(); i++) {
                ProcessorInfo* info = g_processors.get(i);
                info->running = true;
                info->current_thread = nullptr;
                info->ticks = 0;
            }
#ifdef __x86_64__
            x86_64_DisableInterrupts(); // task switch code re-enables them
#endif
            g_scheduler_running = true; // this means that the scheduler is actually running
            ProcessorInfo* current_processor = nullptr;
            for (uint64_t i = 0; i < g_processors.getCount(); i++) {
                ProcessorInfo* info = g_processors.get(i);
                PickNext(info);
#ifdef __x86_64__
                x86_64_LocalAPIC* LAPIC = info->processor->GetLocalAPIC();
                if (LAPIC == nullptr) {
                    PANIC("FATAL: Processor does not have a Local APIC!");
                }
                if (LAPIC->GetID() == GetCurrentProcessorID())
                    current_processor = info;
                else
                    info->start_allowed = 1;
#endif
            }
            g_processors.unlock();
            if (current_processor == nullptr) {
                PANIC("Scheduler: Invalid current processor. This should not be possible.");
            }
            if (current_processor->current_thread == nullptr) {
                PANIC("Scheduler: No available threads. This means all threads have ended and there is nothing else to run.");
            }
            assert(current_processor->current_thread->GetCPURegisters() != nullptr);
#ifdef __x86_64__
            SetThreadFrame(current_processor, current_processor->current_thread->GetStackRegisterFrame());
            if (current_processor->current_thread->GetParent()->GetPriority() == Priority::KERNEL)
                x86_64_kernel_switch(current_processor->current_thread->GetCPURegisters());
            else
                x86_64_enter_user(current_processor->current_thread->GetCPURegisters());
#endif
            PANIC("Failed to start Scheduler. This should never happen and most likely means the task switch code for the relevant architecture returned.");
        }

        void __attribute__((noreturn)) Next() {
            if (!g_scheduler_running)
                PANIC("Scheduler: Scheduler is not running. This should not be possible.");
            ProcessorInfo* info = GetCurrentProcessorInfo();
            if (info->current_thread == nullptr) {
                PANIC("Scheduler: No available threads. This means all threads have ended and there is nothing else to run.");
            }
            assert(info->current_thread->GetCPURegisters() != nullptr);
            info->ticks = 0;
#ifdef __x86_64__
            SetThreadFrame(info, info->current_thread->GetStackRegisterFrame());
            if (info->current_thread->GetParent()->GetPriority() == Priority::KERNEL)
                x86_64_kernel_switch(info->current_thread->GetCPURegisters());
            else
                x86_64_enter_user(info->current_thread->GetCPURegisters());
#endif
            PANIC("Failed to switch to new thread. This should never happen and most likely means the task switch code for the relevant architecture returned.");
        }

        void Next(void* iregs) {
            if (!g_scheduler_running) {
                PANIC("Scheduler: Scheduler is not running. This should not be possible.");
            }
            ProcessorInfo* info = GetCurrentProcessorInfo();
            if (info->current_thread == nullptr) {
                PANIC("Scheduler: No available threads. This means all threads have ended and there is nothing else to run.");
            }
            assert(info->current_thread->GetCPURegisters() != nullptr);
            info->ticks = 0;
#ifdef __x86_64__
            SetThreadFrame(info, info->current_thread->GetStackRegisterFrame());
            if (info->current_thread->GetParent()->GetPriority() == Priority::KERNEL)
                x86_64_kernel_switch(info->current_thread->GetCPURegisters());
            else {
                x86_64_PrepareNewRegisters((x86_64_Interrupt_Registers*)iregs, info->current_thread->GetCPURegisters());
                return;
            }
#endif
            PANIC("Failed to switch to new thread. This should never happen and most likely means the task switch code for the relevant architecture returned.");
        }

        void __attribute__((noreturn)) Next(Thread* thread) {
            if (!g_scheduler_running)
                PANIC("Scheduler: Scheduler is not running. This should not be possible.");
            ProcessorInfo* info = GetCurrentProcessorInfo();
#ifdef __x86_64__
            x86_64_DisableInterrupts();
#endif
            assert(thread != nullptr);
            assert(thread->GetCPURegisters() != nullptr);
            info->ticks = 0;
#ifdef __x86_64__
            SetThreadFrame(info, thread->GetStackRegisterFrame());
            if (thread->GetParent()->GetPriority() == Priority::KERNEL)
                x86_64_kernel_switch(thread->GetCPURegisters());
            else
                x86_64_enter_user(thread->GetCPURegisters());
#endif
            PANIC("Failed to switch to new thread. This should never happen and most likely means the task switch code for the relevant architecture returned.");
        }

        void End() {
            ProcessorInfo* info = GetCurrentProcessorInfo();
            // TODO: call any destructors or other destruction function
            if (info->current_thread->GetFlags() & CREATE_STACK)
                info->current_thread->GetParent()->GetPageManager()->FreePages((void*)(info->current_thread->GetStack() - KiB(64)));
            info->current_thread = nullptr;
            PickNext(info);
            Next();
        }

        Thread* GetCurrent() {
            if (!g_scheduler_running)
                return nullptr;
            ProcessorInfo* info = GetCurrentProcessorInfo();
            if (!info->running)
                return nullptr;
            return info->current_thread;
        }

        void PickNext(ProcessorInfo* info) {
            if (info == nullptr)
                info = GetCurrentProcessorInfo();
            spinlock_acquire(&g_global_lock);
            if (g_total_threads == 0) {
                PANIC("Scheduler: No available threads. This means all threads have ended and there is nothing else to run.");
            }
            if (g_total_threads == 1 && info->current_thread != nullptr) {
                spinlock_release(&g_global_lock);
                return;
            }
            spinlock_release(&g_global_lock);
            if (info->current_thread != nullptr) {
                switch (info->current_thread->GetParent()->GetPriority()) {
                    case Priority::KERNEL:
                        g_kernel_threads.lock();
                        g_kernel_threads.insert(info->current_thread);
                        g_kernel_threads.unlock();
                        info->kernel_run_count++;
                        break;
                    case Priority::HIGH:
                        g_high_threads.lock();
                        g_high_threads.insert(info->current_thread);
                        g_high_threads.unlock();
                        info->high_run_count++;
                        break;
                    case Priority::NORMAL:
                        g_normal_threads.lock();
                        g_normal_threads.insert(info->current_thread);
                        g_normal_threads.unlock();
                        info->normal_run_count++;
                        break;
                    case Priority::LOW:
                        g_low_threads.lock();
                        g_low_threads.insert(info->current_thread);
                        g_low_threads.unlock();
                        break;
                    default:
                        PANIC("Scheduler: A thread has run with an unknown priority.");
                        return; // unnecessary, but only here to remove compiler warnings
                }
            }
            g_kernel_threads.lock();
            g_high_threads.lock();
            g_normal_threads.lock();
            g_low_threads.lock();
            if ((info->kernel_run_count >= 4 || g_kernel_threads.getCount() == 0) && (g_high_threads.getCount() > 0 || g_normal_threads.getCount() > 0 || g_low_threads.getCount() > 0)) { // time to select a high thread
                g_kernel_threads.unlock(); // we must unlock as soon as possible
                if ((info->high_run_count >= 4 || g_high_threads.getCount() == 0) && (g_normal_threads.getCount() > 0 || g_low_threads.getCount() > 0)) { // time to select a normal thread
                    g_high_threads.unlock();
                    if ((info->normal_run_count >= 4 || g_normal_threads.getCount() == 0) && g_low_threads.getCount() > 0) { // time to select a low thread
                        g_normal_threads.unlock();
                        info->current_thread = g_low_threads.get(0);
                        g_low_threads.remove(UINT64_C(0));
                        g_low_threads.unlock();
                        info->normal_run_count = 0;
                    }
                    else {
                        info->current_thread = g_normal_threads.get(0);
                        g_normal_threads.remove(UINT64_C(0));
                        g_normal_threads.unlock();
                        g_low_threads.unlock();
                    }
                    info->high_run_count = 0;
                }
                else {
                    info->current_thread = g_high_threads.get(0);
                    g_high_threads.remove(UINT64_C(0));
                    g_high_threads.unlock();
                    g_normal_threads.unlock();
                    g_low_threads.unlock();
                }
                info->kernel_run_count = 0;
            }
            else {
                if (g_kernel_threads.getCount() == 0)
                    info->current_thread = nullptr;
                else {
                    info->current_thread = g_kernel_threads.get(0);
                    g_kernel_threads.remove(UINT64_C(0));
                }
                g_kernel_threads.unlock();
                g_high_threads.unlock();
                g_normal_threads.unlock();
                g_low_threads.unlock();
            }
        }


        void TimerTick(void* iregs) {
            ProcessorInfo* info = GetCurrentProcessorInfo();
            info->ticks++;
            for (uint64_t i = 0; i < g_sleeping_threads.getCount(); i++) {
                Thread* thread = g_sleeping_threads.get(i);
                thread->SetRemainingSleepTime(thread->GetRemainingSleepTime() - MS_PER_TICK);
                if (thread->GetRemainingSleepTime() == 0) {
                    thread->SetSleeping(false);
                    g_sleeping_threads.lock();
                    g_sleeping_threads.remove(i);
                    g_sleeping_threads.unlock();
                    ReaddThread(thread);
                    i--;
                }
            }
            if (info->ticks == TICKS_PER_SCHEDULER_CYCLE) {
                info->ticks = 0;
                if (!info->running)
                    return; // Nothing to switch to
                if (info->current_thread == nullptr) {
                    PANIC("Scheduler: No available threads. This means all threads have ended and there is nothing else to run.");
                }
                PickNext(info);
#ifdef __x86_64__
                if (info->current_thread->GetParent()->GetPriority() == Priority::KERNEL)
                    x86_64_IOAPIC_SendEOI();
#endif
                Next(iregs);
            }
        }

        bool GlobalIsRunning() {
            return g_scheduler_running;
        }

        bool isRunning() {
            ProcessorInfo* info = GetCurrentProcessorInfo();
            return info->running;
        }

        void StopGlobal() {
            spinlock_acquire(&g_global_lock);
            g_scheduler_running = false;
            spinlock_release(&g_global_lock);
        }

        void Stop() {
            ProcessorInfo* info = GetCurrentProcessorInfo();
            info->running = false;
        }

        void ResumeGlobal() {
            PANIC("Scheduler: Global resume not supported yet.");
        }

        void Resume() {
            ProcessorInfo* info = GetCurrentProcessorInfo();
            info->running = true;
            if (info->current_thread == nullptr)
                PickNext(info);
            if (info->current_thread == nullptr) {
                PANIC("Scheduler: No available threads. This means all threads have ended and there is nothing else to run.");
            }
            Next();
        }

        void SleepThread(Thread* thread, uint64_t ms) {
            PANIC("Scheduler: sleeping not supported.");
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
                g_processors.lock();
                for (uint64_t i = 0; i < g_processors.getCount(); i++) {
                    ProcessorInfo* info = g_processors.get(i);
                    if (info->current_thread == thread) {
                        info->current_thread = nullptr;
                        found = true;
                        g_total_threads--;
                        PickNext();
                        break;
                    }
                }
                g_processors.unlock();
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
                g_kernel_threads.lock();
                g_kernel_threads.insert(thread);
                g_kernel_threads.unlock();
                break;
            case Priority::HIGH:
                g_high_threads.lock();
                g_high_threads.insert(thread);
                g_high_threads.unlock();
                break;
            case Priority::NORMAL:
                g_normal_threads.lock();
                g_normal_threads.insert(thread);
                g_normal_threads.unlock();
                break;
            case Priority::LOW:
                g_low_threads.lock();
                g_low_threads.insert(thread);
                g_low_threads.unlock();
                break;
            default:
                return;
            }
        }

        int SendSignal(Process* sender, pid_t PID, int signum) {
            if (sender == nullptr)
                return -EFAULT;
            Process* receiver = nullptr;
            for (uint64_t i = 0; i < g_processes.getCount(); i++) {
                if (g_processes.get(i)->GetPID() == PID) {
                    receiver = g_processes.get(i);
                    break;
                }
            }
            if (receiver == nullptr)
                return -EINVAL;
            if (PriorityGreaterThan(receiver->GetPriority(), sender->GetPriority()))
                return -EPERM;
            if (receiver->IsInSignalHandler(signum))
                return -EAGAIN;
            receiver->ReceiveSignal(signum);
            return ESUCCESS;
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
            for (uint64_t i = 0; i < g_processors.getCount(); i++) {
                ProcessorInfo* info = g_processors.get(i);
                fprintf(file, "Processor %d:\n", info->id);
                if (info->current_thread != nullptr) {
                    fprintf(file, "Current Thread:\n");
                    info->current_thread->PrintInfo(file);
                    fputc(file, '\n');
                }
            }
        }
    }
}
