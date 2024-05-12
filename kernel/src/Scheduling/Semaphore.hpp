/*
Copyright (©) 2024  Frosty515

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

#ifndef _SEMAPHORE_HPP
#define _SEMAPHORE_HPP

#include <stdint.h>
#include <spinlock.h>

#include <Data-structures/LinkedList.hpp>

#include "Scheduler.hpp"
#include "Thread.hpp"

namespace Scheduling {

    class Thread;

    class Semaphore {
    public:
        Semaphore(uint32_t value);
        ~Semaphore();
        
        void acquire(Thread* thread);
        void release(Thread* thread);

        void acquire_nolock(Thread* thread);

        void Lock();
        void Unlock();
        
        uint64_t getValue() const;

        bool isHeldByThread(Thread* thread);

        bool isWaiting(Thread* thread);

        int GetID() const;

        void SetID(int id);

    private:
        uint64_t m_value;
        int m_id;
        spinlock_t m_lock;

        Scheduler::ThreadList m_waitingThreads;
        LinkedList::LockableLinkedList<Thread> m_holders;
    };

}

#endif /* _SEMAPHORE_HPP */