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
#include "Data-structures/LinkedList.hpp"
#include "Semaphore.hpp"
#include "arch/x86_64/Scheduling/task.h"
#include "arch/x86_64/interrupts/APIC/IPI.hpp"

#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <spinlock.h>
#include <util.h>

#include <process.h>

#include <HAL/hal.hpp>

#ifdef __x86_64__
#include <arch/x86_64/io.h>
#include <arch/x86_64/Processor.hpp>
#include <arch/x86_64/Stack.hpp>

#include <arch/x86_64/interrupts/APIC/IOAPIC.hpp>

#include <arch/x86_64/Memory/PagingUtil.hpp>

#include <arch/x86_64/Scheduling/taskutil.hpp>
#endif

namespace Scheduling {

    namespace Scheduler {

        ThreadList::ThreadList() : m_start(nullptr), m_end(nullptr), m_count(0), m_lock(0) {

        }

        ThreadList::~ThreadList() {

        }

        void ThreadList::PushBack(Thread* thread) {
            if (m_end != nullptr)
                m_end->SetNextThread(thread);
            thread->SetPreviousThread(m_end);
            m_end = thread;
            if (m_start == nullptr)
                m_start = thread;
            m_count++;
        }

        void ThreadList::PushFront(Thread* thread) {
            thread->SetNextThread(m_start);
            if (m_start != nullptr)
                m_start->SetPreviousThread(thread);
            m_start = thread;
            if (m_end == nullptr)
                m_end = thread;
            m_count++;
        }

        Thread* ThreadList::PopBack() {
            if (m_count == 0)
                return nullptr;
            if (m_count == 1) {
                Thread* thread = m_end;
                m_start = nullptr;
                m_end = nullptr;
                m_count = 0;
                return thread;
            }
            Thread* thread = m_end;
            m_end = m_end->GetPreviousThread();
            m_end->SetNextThread(nullptr);
            thread->SetPreviousThread(nullptr);
            m_count--;
            return thread;
        }

        Thread* ThreadList::PopFront() {
            if (m_count == 0)
                return nullptr;
            if (m_count == 1) {
                Thread* thread = m_start;
                m_start = nullptr;
                m_end = nullptr;
                m_count = 0;
                return thread;
            }
            Thread* thread = m_start;
            m_start = thread->GetNextThread();
            m_start->SetPreviousThread(nullptr);
            thread->SetNextThread(nullptr);
            m_count--;
            return thread;
        }

        bool ThreadList::RemoveThread(Thread* thread) {
            Thread* current = m_start;
            while (current != nullptr) {
                if (current == thread) {
                    if (current == m_start) {
                        m_start = current->GetNextThread();
                        if (m_start != nullptr)
                            m_start->SetPreviousThread(nullptr);
                    }
                    else if (current == m_end) {
                        m_end = current->GetPreviousThread();
                        if (m_end != nullptr)
                            m_end->SetNextThread(nullptr);
                    }
                    else {
                        current->GetPreviousThread()->SetNextThread(current->GetNextThread());
                        current->GetNextThread()->SetPreviousThread(current->GetPreviousThread());
                    }
                    current->SetNextThread(nullptr);
                    current->SetPreviousThread(nullptr);
                    return true;
                }
                current = current->GetNextThread();
            }
            return false;
        }

        Thread* ThreadList::Get(uint64_t index) {
            if (index >= m_count)
                return nullptr;
            Thread* current = m_start;
            for (uint64_t i = 0; i < index; i++)
                current = current->GetNextThread();
            return current;
        }

        bool ThreadList::Contains(Thread* thread) {
            Thread* current = m_start;
            while (current != nullptr) {
                if (current == thread)
                    return true;
                current = current->GetNextThread();
            }
            return false;
        }

        void ThreadList::EnumerateThreads(void(*callback)(Thread* thread, void* data), void* data) {
            Thread* current = m_start;
            while (current != nullptr) {
                callback(current, data);
                current = current->GetNextThread();
            }
        }

        uint64_t ThreadList::GetCount() const {
            return m_count;
        }

        void ThreadList::Lock() const {
            spinlock_acquire(&m_lock);
        }

        void ThreadList::Unlock() const {
            spinlock_release(&m_lock);
        }


        ProcessorInfo g_BSPInfo;

        LinkedList::LockableLinkedList<ProcessorInfo> g_processors;
        LinkedList::LockableLinkedList<Process> g_processes;
        LinkedList::LockableLinkedList<Semaphore> g_semaphores;
        ThreadList g_kernel_threads;
        ThreadList g_high_threads;
        ThreadList g_normal_threads;
        ThreadList g_low_threads;
        ThreadList g_idle_threads;
        ThreadList g_sleeping_threads;
        uint64_t g_total_threads = 0;
        pid_t g_NextPID = 0;
        int g_NextSemaphoreID = 0;
        bool g_scheduler_running = false;
        spinlock_new(g_global_lock);

        void ClearGlobalData() {
            g_processors.unlock();
            g_processes.unlock();
            g_total_threads = 0;
            g_NextPID = 0;
            g_NextSemaphoreID = 0;
            g_scheduler_running = false;
            spinlock_init(&g_global_lock);

            g_kernel_threads = ThreadList();
            g_high_threads = ThreadList();
            g_normal_threads = ThreadList();
            g_low_threads = ThreadList();
            g_sleeping_threads = ThreadList();
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
            g_processes.lock();
            Priority thread_priority = thread->GetParent()->GetPriority();
            if (g_processes.getIndex(thread->GetParent()) == UINT64_MAX)
                AddProcess(thread->GetParent());
            g_processes.unlock();
            CPU_Registers* regs = thread->GetCPURegisters();
            memset(regs, 0, sizeof(CPU_Registers));
#ifdef __x86_64__
            if ((thread->GetFlags() & CREATE_STACK)) {
                if (thread->GetParent()->GetPriority() == Priority::KERNEL && thread->GetKernelStack() != 0) {
                    thread->SetStack(thread->GetKernelStack());
                    regs->RSP = thread->GetStack();
                }
                else {
                    x86_64_GetNewStack(thread->GetParent()->GetPageManager(), regs, KiB(64));
                    thread->GetParent()->SyncRegion();
                    thread->SetStack(regs->RSP);
                }
            }
            else
                regs->RSP = thread->GetStack();
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
                g_kernel_threads.Lock();
                g_kernel_threads.PushBack(thread);
                g_kernel_threads.Unlock();
            }
            else {
#ifdef __x86_64__
                regs->CS = 0x23; // User Code Segment
                regs->DS = 0x1b; // User Data Segment
#endif
                switch (thread_priority) {
                    case Priority::HIGH:
                        g_high_threads.Lock();
                        g_high_threads.PushBack(thread);
                        g_high_threads.Unlock();
                        break;
                    case Priority::NORMAL:
                        g_normal_threads.Lock();
                        g_normal_threads.PushBack(thread);
                        g_normal_threads.Unlock();
                        break;
                    case Priority::LOW:
                        g_low_threads.Lock();
                        g_low_threads.PushBack(thread);
                        g_low_threads.Unlock();
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
                    g_kernel_threads.Lock();
                    success = g_kernel_threads.RemoveThread(thread);
                    g_kernel_threads.Unlock();
                    break;
                }
                case Priority::HIGH: {
                    g_high_threads.Lock();
                    success = g_high_threads.RemoveThread(thread);
                    g_high_threads.Unlock();
                    break;
                }
                case Priority::NORMAL: {
                    g_normal_threads.Lock();
                    success = g_normal_threads.RemoveThread(thread);
                    g_normal_threads.Unlock();
                    break;
                }
                case Priority::LOW: {
                    g_low_threads.Lock();
                    success = g_low_threads.RemoveThread(thread);
                    g_low_threads.Unlock();
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
                        PickNext(proc);
                        break;
                    }
                }
                g_processors.unlock();
            }
            else
                g_total_threads--;
        }

        void __attribute__((no_sanitize("undefined"))) AddProcessor(Processor* processor) {
            /*__asm__ volatile("cli");
            while (true) {
                __asm__ volatile ("hlt");
            }
            */
            ProcessorInfo* info = new ProcessorInfo();
            info->processor = processor;
            info->kernel_run_count = 0;
            info->high_run_count = 0;
            info->normal_run_count = 0;
            info->current_thread = nullptr;
            info->running = false;
            info->ticks = 0;
            info->start_allowed = 0;
            g_processors.lock();
            info->id = g_processors.getCount();
            g_processors.insert(info);
            g_processors.unlock();
#ifdef __x86_64__
            x86_64_set_kernel_gs_base((uint64_t)info);
            x86_64_LocalAPIC* LAPIC = processor->GetLocalAPIC();
            if (LAPIC == nullptr)
                return; // should be unreachable
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
            x86_64_context_switch(info->current_thread->GetCPURegisters());
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

        ProcessorInfo* GetProcessorInfo(uint8_t ID) {
            g_processors.lock();
            for (uint64_t i = 0; i < g_processors.getCount(); i++) {
                ProcessorInfo* info = g_processors.get(i);
#ifdef __x86_64__
                x86_64_LocalAPIC* LAPIC = info->processor->GetLocalAPIC();
                if (LAPIC == nullptr)
                    continue;
                if (LAPIC->GetID() == ID) {
                    g_processors.unlock();
                    return info;
                }
#endif
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

        uint8_t GetProcessorCount() {
            g_processors.lock();
            uint8_t count = g_processors.getCount();
            g_processors.unlock();
            return count;
        }

        void EnumerateProcessors(void(*callback)(ProcessorInfo* info, void* data), void* data) {
            g_processors.lock();
            for (uint64_t i = 0; i < g_processors.getCount(); i++) {
                ProcessorInfo* info = g_processors.get(i);
                callback(info, data);
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
#ifdef __x86_64__
            x86_64_DisableInterrupts(); // task switch code re-enables them
#endif
            g_processors.lock();
            for (uint64_t i = 0; i < g_processors.getCount(); i++) {
                ProcessorInfo* info = g_processors.get(i);
                info->running = true;
                info->current_thread = nullptr;
                info->ticks = 0;
            }
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
            x86_64_context_switch(current_processor->current_thread->GetCPURegisters());
#endif
            PANIC("Failed to start Scheduler. This should never happen and most likely means the task switch code for the relevant architecture returned.");
        }

        void __attribute__((noreturn)) Next() {
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
            x86_64_context_switch(info->current_thread->GetCPURegisters());
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
            x86_64_PrepareNewRegisters((x86_64_Interrupt_Registers*)iregs, info->current_thread->GetCPURegisters());
            return;
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
            x86_64_context_switch(thread->GetCPURegisters());
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
                if (info->current_thread->IsIdle()) {
                    g_idle_threads.Lock();
                    g_idle_threads.PushBack(info->current_thread);
                    g_idle_threads.Unlock();
                }
                else {
                    switch (info->current_thread->GetParent()->GetPriority()) {
                        case Priority::KERNEL:
                            g_kernel_threads.Lock();
                            g_kernel_threads.PushBack(info->current_thread);
                            g_kernel_threads.Unlock();
                            info->kernel_run_count++;
                            break;
                        case Priority::HIGH:
                            g_high_threads.Lock();
                            g_high_threads.PushBack(info->current_thread);
                            g_high_threads.Unlock();
                            info->high_run_count++;
                            break;
                        case Priority::NORMAL:
                            g_normal_threads.Lock();
                            g_normal_threads.PushBack(info->current_thread);
                            g_normal_threads.Unlock();
                            info->normal_run_count++;
                            break;
                        case Priority::LOW:
                            g_low_threads.Lock();
                            g_low_threads.PushBack(info->current_thread);
                            g_low_threads.Unlock();
                            break;
                        default:
                            PANIC("Scheduler: A thread has run with an unknown priority.");
                            return; // unnecessary, but only here to remove compiler warnings
                    }
                }
            }
            g_kernel_threads.Lock();
            g_high_threads.Lock();
            g_normal_threads.Lock();
            g_low_threads.Lock();
            if ((info->kernel_run_count >= 4 || g_kernel_threads.GetCount() == 0) && (g_high_threads.GetCount() > 0 || g_normal_threads.GetCount() > 0 || g_low_threads.GetCount() > 0)) { // time to select a high thread
                g_kernel_threads.Unlock(); // we must unlock as soon as possible
                if ((info->high_run_count >= 4 || g_high_threads.GetCount() == 0) && (g_normal_threads.GetCount() > 0 || g_low_threads.GetCount() > 0)) { // time to select a normal thread
                    g_high_threads.Unlock();
                    if ((info->normal_run_count >= 4 || g_normal_threads.GetCount() == 0) && g_low_threads.GetCount() > 0) { // time to select a low thread
                        g_normal_threads.Unlock();
                        info->current_thread = g_low_threads.PopFront();
                        g_low_threads.Unlock();
                        info->normal_run_count = 0;
                    }
                    else {
                        g_low_threads.Unlock();
                        info->current_thread = g_normal_threads.PopFront();
                        g_normal_threads.Unlock();
                    }
                    info->high_run_count = 0;
                }
                else {
                    g_normal_threads.Unlock();
                    g_low_threads.Unlock();
                    info->current_thread = g_high_threads.PopFront();
                    g_high_threads.Unlock();
                }
                info->kernel_run_count = 0;
            }
            else {
                g_high_threads.Unlock();
                g_normal_threads.Unlock();
                g_low_threads.Unlock();
                if (g_kernel_threads.GetCount() == 0) {
                    g_kernel_threads.Unlock();
                    g_idle_threads.Lock();
                    if (g_idle_threads.GetCount() > 0)
                        info->current_thread = g_idle_threads.PopFront();
                    else
                        info->current_thread = nullptr;
                    g_idle_threads.Unlock();
                }
                else {
                    info->current_thread = g_kernel_threads.PopFront();
                    g_kernel_threads.Unlock();
                }
            }
        }


        void TimerTick(void* iregs) {
            ProcessorInfo* info = GetCurrentProcessorInfo();
            info->ticks++;
            g_sleeping_threads.Lock();
            for (uint64_t i = 0; i < g_sleeping_threads.GetCount(); i++) { // can't use EnumerateThreads here, as we need to modify the list
                Thread* thread = g_sleeping_threads.Get(i);
                thread->SetRemainingSleepTime(thread->GetRemainingSleepTime() - MS_PER_TICK);
                if (thread->GetRemainingSleepTime() == 0) {
                    thread->SetSleeping(false);
                    g_sleeping_threads.RemoveThread(thread);
                    ReaddThread(thread);
                    i--;
                }
            }
            g_sleeping_threads.Unlock();
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
            assert(thread != nullptr);
            // Remove the thread
            bool found = false;
            switch (thread->GetParent()->GetPriority()) {
            case Priority::KERNEL: {
                g_kernel_threads.Lock();
                found = g_kernel_threads.RemoveThread(thread);
                g_kernel_threads.Unlock();
                break;
            }
            case Priority::HIGH: {
                g_high_threads.Lock();
                found = g_high_threads.RemoveThread(thread);
                g_high_threads.Unlock();
                break;
            }
            case Priority::NORMAL: {
                g_normal_threads.Lock();
                found = g_normal_threads.RemoveThread(thread);
                g_normal_threads.Unlock();
                break;
            }
            case Priority::LOW: {
                g_low_threads.Lock();
                found = g_low_threads.RemoveThread(thread);
                g_low_threads.Unlock();
                break;
            }
            default:
                return;
            }
            if (!found) {
                ProcessorInfo* current = GetCurrentProcessorInfo();
                g_processors.lock();
                for (uint64_t i = 0; i < g_processors.getCount(); i++) {
                    ProcessorInfo* info = g_processors.get(i);
                    if (info->current_thread == thread) {
                        info->current_thread = nullptr;
                        found = true;
                        g_total_threads--;
                        thread->SetSleeping(true);
                        thread->SetRemainingSleepTime(ALIGN_UP(ms, MS_PER_TICK));
                        assert(thread->GetCPURegisters() != nullptr);
                        //thread->GetCPURegisters()->RIP = (uint64_t)return_address;
                        g_sleeping_threads.Lock();
                        g_sleeping_threads.PushBack(thread);
                        g_sleeping_threads.Unlock();
                        PickNext(info);
                        g_processors.unlock();

                        if (info->id == current->id) {
                            Next();
                        }
                        else {
#ifdef __x86_64__
                            x86_64_LocalAPIC* LAPIC = info->processor->GetLocalAPIC();
                            x86_64_IssueIPI(x86_64_IPI_DestinationShorthand::NoShorthand, LAPIC->GetID(), x86_64_IPI_Type::NextThread, 0, true);
#endif
                        }
                        break;
                    }
                }
                g_processors.unlock();
            }
            else {
                thread->SetSleeping(true);
                thread->SetRemainingSleepTime(ALIGN_UP(ms, MS_PER_TICK));
                assert(thread->GetCPURegisters() != nullptr);
                //thread->GetCPURegisters()->RIP = (uint64_t)return_address;
                g_sleeping_threads.Lock();
                g_sleeping_threads.PushBack(thread);
                g_sleeping_threads.Unlock();
            }
        }

        void ReaddThread(Thread* thread) {
            assert(thread != nullptr);
            switch (thread->GetParent()->GetPriority()) {
            case Priority::KERNEL:
                g_kernel_threads.Lock();
                g_kernel_threads.PushBack(thread);
                g_kernel_threads.Unlock();
                break;
            case Priority::HIGH:
                g_high_threads.Lock();
                g_high_threads.PushBack(thread);
                g_high_threads.Unlock();
                break;
            case Priority::NORMAL:
                g_normal_threads.Lock();
                g_normal_threads.PushBack(thread);
                g_normal_threads.Unlock();
                break;
            case Priority::LOW:
                g_low_threads.Lock();
                g_low_threads.PushBack(thread);
                g_low_threads.Unlock();
                break;
            default:
                return;
            }
        }

        int SendSignal(Process* sender, pid_t PID, int signum) {
            if (sender == nullptr)
                return -EFAULT;
            Process* receiver = nullptr;
            g_processes.lock();
            for (uint64_t i = 0; i < g_processes.getCount(); i++) {
                if (g_processes.get(i)->GetPID() == PID) {
                    receiver = g_processes.get(i);
                    break;
                }
            }
            g_processes.unlock();
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
            g_kernel_threads.Lock();
            if (g_kernel_threads.GetCount() > 0) {
                fprintf(file, "Kernel Threads:\n");
                g_kernel_threads.EnumerateThreads([](Thread* thread, void* raw_file) {
                    fd_t file = *reinterpret_cast<fd_t*>(raw_file);
                    thread->PrintInfo(file);
                    fputc(file, '\n');
                }, &file);
            }
            g_kernel_threads.Unlock();
            g_high_threads.Lock();
            if (g_high_threads.GetCount() > 0) {
                fprintf(file, "High Threads:\n");
                g_high_threads.EnumerateThreads([](Thread* thread, void* raw_file) {
                    fd_t file = *reinterpret_cast<fd_t*>(raw_file);
                    thread->PrintInfo(file);
                    fputc(file, '\n');
                }, &file);
            }
            g_high_threads.Unlock();
            g_normal_threads.Lock();
            if (g_normal_threads.GetCount() > 0) {
                fprintf(file, "Normal Threads:\n");
                g_normal_threads.EnumerateThreads([](Thread* thread, void* raw_file) {
                    fd_t file = *reinterpret_cast<fd_t*>(raw_file);
                    thread->PrintInfo(file);
                    fputc(file, '\n');
                }, &file);
            }
            g_normal_threads.Unlock();
            g_low_threads.Lock();
            if (g_low_threads.GetCount() > 0) {
                fprintf(file, "Low Threads:\n");
                g_low_threads.EnumerateThreads([](Thread* thread, void* raw_file) {
                    fd_t file = *reinterpret_cast<fd_t*>(raw_file);
                    thread->PrintInfo(file);
                    fputc(file, '\n');
                }, &file);
            }
            g_low_threads.Unlock();
            g_sleeping_threads.Lock();
            if (g_sleeping_threads.GetCount() > 0) {
                fprintf(file, "Sleeping Threads:\n");
                g_sleeping_threads.EnumerateThreads([](Thread* thread, void* raw_file) {
                    fd_t file = *reinterpret_cast<fd_t*>(raw_file);
                    thread->PrintInfo(file);
                    fputc(file, '\n');
                }, &file);
            }
            g_sleeping_threads.Unlock();
            g_processors.lock();
            for (uint64_t i = 0; i < g_processors.getCount(); i++) {
                ProcessorInfo* info = g_processors.get(i);
                fprintf(file, "Processor %d:\n", info->id);
                if (info->current_thread != nullptr) {
                    fprintf(file, "Current Thread:\n");
                    info->current_thread->PrintInfo(file);
                    fputc(file, '\n');
                }
            }
            g_processors.unlock();
        }

        void ForceUnlockEverything() {
            g_kernel_threads.Unlock();
            g_high_threads.Unlock();
            g_normal_threads.Unlock();
            g_low_threads.Unlock();
            g_sleeping_threads.Unlock();
            g_processors.unlock();
            g_processes.unlock();
        }

        void AddIdleThread(Thread* thread) {
            /*
            Idle thread properties:
            - No entry point
            - Always in kernel mode
            - Don't end
            - Only run if nothing else can be run on that CPU
            */
            assert(thread != nullptr);
            g_processes.lock();
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
            regs->RIP = (uint64_t)x86_64_idle_loop;
            regs->RFLAGS = (1 << 9) | (1 << 1); // IF Flag and Reserved (always 1)
            regs->CR3 = (uint64_t)(thread->GetParent()->GetPageManager()->GetPageTable().GetRootTablePhysical()) & 0x000FFFFFFFFFF000;
            regs->CS = 0x08; // Kernel Code Segment
            regs->DS = 0x10; // Kernel Data Segment
#else
#error Unkown Architecture
#endif
            thread->SetIdle(true);
            g_idle_threads.Lock();
            g_idle_threads.PushBack(thread);
            g_idle_threads.Unlock();
            spinlock_acquire(&g_global_lock);
            g_total_threads++;
            spinlock_release(&g_global_lock);
        }

        int RegisterSemaphore(Semaphore* semaphore) {
            if (semaphore == nullptr)
                return -EINVAL;

            g_semaphores.lock();
            g_semaphores.insert(semaphore);
            g_semaphores.unlock();

            spinlock_acquire(&g_global_lock);
            int ID = g_NextSemaphoreID;
            semaphore->SetID(ID);
            g_NextSemaphoreID++;
            spinlock_release(&g_global_lock);

            return ID;
        }

        Semaphore* GetSemaphore(int ID) {
            if (ID < 0)
                return nullptr;

            g_semaphores.lock();
            for (uint64_t i = 0; i < g_semaphores.getCount(); i++) {
                Semaphore* semaphore = g_semaphores.get(i);
                if (semaphore->GetID() == ID) {
                    g_semaphores.unlock();
                    return semaphore;
                }
            }
            g_semaphores.unlock();

            return nullptr;
        }

        int UnregisterSemaphore(int ID) {
            if (ID < 0)
                return -EINVAL;

            g_semaphores.lock();
            for (uint64_t i = 0; i < g_semaphores.getCount(); i++) {
                Semaphore* semaphore = g_semaphores.get(i);
                if (semaphore->GetID() == ID) {
                    g_semaphores.remove(i);
                    g_semaphores.unlock();
                    return ESUCCESS;
                }
            }
            g_semaphores.unlock();
            return -EINVAL;
        }

        int UnregisterSemaphore(Semaphore* semaphore) {
            if (semaphore == nullptr)
                return -EINVAL;
            
            g_semaphores.lock();
            uint64_t index = g_semaphores.getIndex(semaphore);
            if (index == UINT64_MAX) {
                g_semaphores.unlock();
                return -EINVAL;
            }
            g_semaphores.remove(index);
            g_semaphores.unlock();

            return ESUCCESS;
        }
    }
}
