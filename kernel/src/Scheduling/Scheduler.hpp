#ifndef _KERNEL_SCHEDULER_HPP
#define _KERNEL_SCHEDULER_HPP

#include <stdint.h>
#include <stddef.h>

#include "Process.hpp"
#include "Thread.hpp"

#include <Data-structures/LinkedList.hpp>

#define TICKS_PER_SCHEDULER_CYCLE 4

namespace Scheduling {


    
    class Scheduler {
    public:
        Scheduler();
        ~Scheduler();

        void AddProcess(Process* process);
        void ScheduleThread(Thread* thread);

        void __attribute__((noreturn)) Start();

        //void __attribute__((noreturn)) Next();
        CPU_Registers* Next(CPU_Registers* regs);

    private:
        LinkedList::SimpleLinkedList<Process*> m_processes;
        LinkedList::SimpleLinkedList<Thread*> m_kernel_threads;
        bool m_running;
    };

    extern Scheduler* g_Scheduler;

    CPU_Registers* TimerTick(CPU_Registers* regs);
}

#endif /* _KERNEL_SCHEDULER_HPP */