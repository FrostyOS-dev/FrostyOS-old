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

#include "Semaphore.hpp"
#include "Scheduler.hpp"

#include <spinlock.h>

#include <HAL/hal.hpp>

#ifdef __x86_64__
#include <arch/x86_64/Scheduling/task.h>
#include <arch/x86_64/Scheduling/taskutil.hpp>
#endif

namespace Scheduling {

    Semaphore::Semaphore(uint32_t value) : m_value(value), m_id(-1), m_lock(0), m_waitingThreads(), m_holders() {

    }

    Semaphore::~Semaphore() {
        m_waitingThreads.Lock();
        while (m_waitingThreads.GetCount() > 0) {
            Thread* thread = m_waitingThreads.PopFront();
            thread->SetBlocked(false);
            Scheduler::ReaddThread(thread);
        }
        m_waitingThreads.Unlock();
    }

    void Semaphore::acquire(Thread* thread) {
        spinlock_acquire(&m_lock);

        if (m_value == 0) {
            m_waitingThreads.Lock();
            m_waitingThreads.PushBack(thread);
            m_waitingThreads.Unlock();
            Thread* current = Scheduler::GetCurrent();
            Scheduler::RemoveThread(thread);
            thread->SetBlocked(true);
            if (current == thread) {
                spinlock_release(&m_lock);
                Scheduler::PickNext();
                Scheduler::Next();
            }
            else {
                Scheduler::EnumerateProcessors([](Scheduler::ProcessorInfo* info, void* data) {
                    if (info->current_thread == (Thread*)data) {
                        info->current_thread = nullptr;
                        PickNext(info);
#ifdef __x86_64__
                        x86_64_LocalAPIC* LAPIC = info->processor->GetLocalAPIC();
                        x86_64_IssueIPI(x86_64_IPI_DestinationShorthand::NoShorthand, LAPIC->GetID(), x86_64_IPI_Type::NextThread, 0, true);
#endif
                    }
                }, thread);
            }
        }
        else {
            m_value--;
            m_holders.lock();
            m_holders.insert(thread);
            m_holders.unlock();
        }

        spinlock_release(&m_lock);
    }

    void Semaphore::release(Thread* thread) {
        m_holders.lock();
        if (m_holders.getIndex(thread) != UINT64_MAX) {
            m_holders.remove(thread);
            m_holders.unlock();
            spinlock_acquire(&m_lock);
            m_value++;
            spinlock_release(&m_lock);

            m_waitingThreads.Lock();
            if (m_waitingThreads.GetCount() > 0) {
                Thread* nextThread = m_waitingThreads.PopFront();
                m_waitingThreads.Unlock();
                spinlock_acquire(&m_lock);
                m_value--;
                spinlock_release(&m_lock);
                m_holders.lock();
                m_holders.insert(nextThread);
                m_holders.unlock();
                nextThread->SetBlocked(false);
                Scheduler::ReaddThread(nextThread);
            }
            else
                m_waitingThreads.Unlock();
        }
        else
            m_holders.unlock();
    }

    void Semaphore::acquire_nolock(Thread* thread) {
        if (m_value != 0) {
            m_value--;
            m_holders.lock();
            m_holders.insert(thread);
            m_holders.unlock();
        }
    }

    void Semaphore::Lock() {
        spinlock_acquire(&m_lock);
    }

    void Semaphore::Unlock() {
        spinlock_release(&m_lock);
    }

    uint64_t Semaphore::getValue() const {
        return m_value;
    }

    bool Semaphore::isHeldByThread(Thread* thread) {
        m_holders.lock();
        bool result = m_holders.getIndex(thread) != UINT64_MAX;
        m_holders.unlock();
        return result;
    }

    bool Semaphore::isWaiting(Thread* thread) {
        m_waitingThreads.Lock();
        bool result = m_waitingThreads.Contains(thread);
        m_waitingThreads.Unlock();
        return result;
    }

    int Semaphore::GetID() const {
        return m_id;
    }

    void Semaphore::SetID(int id) {
        m_id = id;
    }

}