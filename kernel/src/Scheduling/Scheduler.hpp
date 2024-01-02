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

// The scheduler needs to switch task every 40ms and the timer runs at 200Hz (or a tick every 5ms), so we need to switch every 8 ticks.

#define MS_PER_SCHEDULER_CYCLE 40
#define TICKS_PER_SCHEDULER_CYCLE MS_PER_SCHEDULER_CYCLE / MS_PER_TICK

namespace Scheduling {

    namespace Scheduler {

        void AddProcess(Process* process);
        void RemoveProcess(Process* process);
        void ScheduleThread(Thread* thread);
        void RemoveThread(Thread* thread);

        void __attribute__((noreturn)) Start();

        void __attribute__((noreturn)) Next();
        void Next(void* iregs);
        void __attribute__((noreturn)) Next(Thread* thread);

        // Called when a thread ends. Responsible for cleanup
        void End();

        void PickNext();

        Thread* GetCurrent();

        void TimerTick(void* iregs); // Only to be called in timer IRQ

        bool isRunning();

        void Stop();
        void Resume();

        void SleepThread(Thread* thread, uint64_t ms);

        void ReaddThread(Thread* thread);

        int SendSignal(Process* sender, pid_t PID, int signum);

        void PrintThreads(fd_t file);
    }

}

#endif /* _KERNEL_SCHEDULER_HPP */