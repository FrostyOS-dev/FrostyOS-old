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

#ifndef _SYNCHRONISATION_H
#define _SYNCHRONISATION_H

#include "syscall.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef uint64_t spinlock_t;

#ifndef _IN_KERNEL

static int create_semaphore(uint64_t count) {
    return system_call(SC_CREATE_SEMAPHORE, count, 0, 0);
}

static int destroy_semaphore(int semaphore) {
    return system_call(SC_DESTROY_SEMAPHORE, semaphore, 0, 0);
}

static int acquire_semaphore(int semaphore) {
    return system_call(SC_ACQUIRE_SEMAPHORE, semaphore, 0, 0);
}

static int release_semaphore(int semaphore) {
    return system_call(SC_RELEASE_SEMAPHORE, semaphore, 0, 0);
}

static int create_mutex() {
    return system_call(SC_CREATE_MUTEX, 0, 0, 0);
}

static int destroy_mutex(int mutex) {
    return system_call(SC_DESTROY_MUTEX, mutex, 0, 0);
}

static int acquire_mutex(int mutex) {
    return system_call(SC_ACQUIRE_MUTEX, mutex, 0, 0);
}

static int release_mutex(int mutex) {
    return system_call(SC_RELEASE_MUTEX, mutex, 0, 0);
}

#else

int create_semaphore(uint64_t count);
int destroy_semaphore(int semaphore);
int acquire_semaphore(int semaphore);
int release_semaphore(int semaphore);

int create_mutex();
int destroy_mutex(int mutex);
int acquire_mutex(int mutex);
int release_mutex(int mutex);

#endif /* _IN_KERNEL */

#ifdef __cplusplus
}
#endif

#endif /* _SYNCHRONISATION_H */