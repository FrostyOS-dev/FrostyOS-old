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
        CPU_Registers* Next(CPU_Registers* regs);

        // Called when a thread ends. Responsible for cleanup
        void End();

        CPU_Registers* TimerTick(CPU_Registers* regs);
    }

}

#endif /* _KERNEL_SCHEDULER_HPP */