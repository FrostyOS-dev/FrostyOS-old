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

#include <mutex.h>

#include <SystemCalls/Synchronisation.hpp>

int createMutex() {
    return sys_createMutex();
}

int acquireMutex(int ID) {
    return sys_acquireMutex(ID);
}

int releaseMutex(int ID) {
    return sys_releaseMutex(ID);
}

int destroyMutex(int ID) {
    return sys_destroyMutex(ID);
}
