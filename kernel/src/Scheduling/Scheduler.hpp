#ifndef _KERNEL_SCHEDULER_HPP
#define _KERNEL_SCHEDULER_HPP

#include <stdint.h>
#include <stddef.h>

#include "Process.hpp"
#include "Thread.hpp"

#include <Data-structures/LinkedList.hpp>

#define TICKS_PER_SCHEDULER_CYCLE 4

namespace Scheduling {

    namespace Scheduler {

        void AddProcess(Process* process);
        void ScheduleThread(Thread* thread);

        void __attribute__((noreturn)) Start();

        void __attribute__((noreturn)) Next();
        void __attribute__((noreturn)) Next(Thread* thread);

        // Called when a thread ends. Responsible for cleanup
        void End();

        Thread* GetCurrent();

        void TimerTick(); // Only to be called in timer IRQ

        bool isRunning();

        void Stop();
    }

}

#endif /* _KERNEL_SCHEDULER_HPP */