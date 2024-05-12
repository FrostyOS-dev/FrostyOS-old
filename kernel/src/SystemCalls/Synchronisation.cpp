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

#include "Synchronisation.hpp"

#ifdef __x86_64__
#include <arch/x86_64/Scheduling/taskutil.hpp>
#endif

#include <errno.h>

#include <Scheduling/Scheduler.hpp>
#include <Scheduling/Semaphore.hpp>

int sys_createSemaphore(int value) {
    using namespace Scheduling;
    Semaphore* semaphore = new Semaphore(value);
    if (semaphore == nullptr)
        return -ENOMEM;

    int ID = Scheduler::RegisterSemaphore(semaphore);
    if (ID < 0)
        delete semaphore;
    return ID;
}

int sys_acquireSemaphore(int ID) {
    Scheduling::Semaphore* semaphore = Scheduling::Scheduler::GetSemaphore(ID);
    if (semaphore == nullptr)
        return -EINVAL;

    Scheduling::Thread* thread = Scheduling::Scheduler::GetCurrent();

    semaphore->Lock();
    if (semaphore->getValue() == 0) {
        semaphore->Unlock();
        semaphore_acquire(semaphore, thread);
    }
    else {
        semaphore->acquire_nolock(thread);
        semaphore->Unlock();
    }

    return ESUCCESS;
}

int sys_releaseSemaphore(int ID) {
    Scheduling::Semaphore* semaphore = Scheduling::Scheduler::GetSemaphore(ID);
    if (semaphore == nullptr)
        return -EINVAL;

    Scheduling::Thread* thread = Scheduling::Scheduler::GetCurrent();

    semaphore->release(thread);


    return ESUCCESS;
}

int sys_destroySemaphore(int ID) {
    Scheduling::Semaphore* semaphore = Scheduling::Scheduler::GetSemaphore(ID);
    if (semaphore == nullptr)
        return -EINVAL;

    Scheduling::Scheduler::UnregisterSemaphore(ID);

    delete semaphore;

    return ESUCCESS;
}

int sys_createMutex() {
    return sys_createSemaphore(1);
}

int sys_acquireMutex(int ID) {
    return sys_acquireSemaphore(ID);
}

int sys_releaseMutex(int ID) {
    return sys_releaseSemaphore(ID);
}

int sys_destroyMutex(int ID) {
    return sys_destroySemaphore(ID);
}
