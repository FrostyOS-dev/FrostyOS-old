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

#ifndef _KERNEL_SCHEDULER_HPP
#define _KERNEL_SCHEDULER_HPP

#include <stdint.h>
#include <stddef.h>

#include <HAL/time.h>

#include "Process.hpp"
#include "Thread.hpp"

#include <Data-structures/LinkedList.hpp>

#ifdef __x86_64__
#include <arch/x86_64/Processor.hpp>
#endif

// The scheduler needs to switch task every 40ms and the timer runs at 1kHz (or a tick every 1ms), so we need to switch every 40 ticks.

#define MS_PER_SCHEDULER_CYCLE 40
#define TICKS_PER_SCHEDULER_CYCLE MS_PER_SCHEDULER_CYCLE / MS_PER_TICK


namespace Scheduling {

    namespace Scheduler {

        struct ProcessorInfo {
            Processor* processor;
            uint64_t id;
            Thread::Register_Frame thread_metadata;
            uint8_t kernel_run_count; // the amount of times a kernel thread has been run in a row
            uint8_t high_run_count;   // the amount of times a high thread has been run in a row
            uint8_t normal_run_count; // the amount of times a normal thread has been run in a row
            Thread* current_thread;
            bool running;
            size_t ticks;
            uint32_t start_allowed; // when this is locked, the processor is not allowed to run anything
        } __attribute__((packed));

        void ClearGlobalData();

        void InitBSPInfo();

        void AddProcess(Process* process);
        void RemoveProcess(Process* process);
        void ScheduleThread(Thread* thread);
        void RemoveThread(Thread* thread);

        void AddProcessor(Processor* processor);
        Processor* GetProcessor(uint8_t ID);
        ProcessorInfo* GetProcessorInfo(Processor* processor);
        void RemoveProcessor(Processor* processor);

        void SetThreadFrame(ProcessorInfo* info, Thread::Register_Frame* frame);

        void InitProcessorTimers();

        void __attribute__((noreturn)) Start();

        void __attribute__((noreturn)) Next();
        void Next(void* iregs);
        void __attribute__((noreturn)) Next(Thread* thread);

        // Called when a thread ends. Responsible for cleanup
        void End();

        void PickNext(ProcessorInfo* info = nullptr); // info points to the CPU which we want to pick the next thread for. If nullptr, the current CPU is used.

        Thread* GetCurrent();

        void TimerTick(void* iregs); // Only to be called in timer IRQ

        // Is the scheduler running globally.
        bool GlobalIsRunning();

        // Is the scheduler running on this CPU
        bool isRunning();

        void Stop();
        void Resume();

        void StopGlobal();
        void ResumeGlobal();

        void SleepThread(Thread* thread, uint64_t ms);

        void ReaddThread(Thread* thread);

        int SendSignal(Process* sender, pid_t PID, int signum);

        void PrintThreads(fd_t file);
    }

}

#endif /* _KERNEL_SCHEDULER_HPP */