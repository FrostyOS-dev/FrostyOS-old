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

#include <semaphore.h>

#include <SystemCalls/Synchronisation.hpp>

// For all of these, we can actually just use the system calls directly, as they are already implemented in the kernel.

extern "C" int createSemaphore(int value) {
    return sys_createSemaphore(value);
}

extern "C" int acquireSemaphore(int ID) {
    return sys_acquireSemaphore(ID);
}

extern "C" int releaseSemaphore(int ID) {
    return sys_releaseSemaphore(ID);
}

extern "C" int destroySemaphore(int ID) {
    return sys_destroySemaphore(ID);
}
