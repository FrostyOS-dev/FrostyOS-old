/*
Copyright (©) 2023  Frosty515

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

#include <sys/memory.h>
#include <sys/syscall.h>
#include <util.h>
#include <errno.h>

void* mmap(unsigned long size, unsigned long perms, void* addr) {
    long new_addr = (long)system_call(SC_MMAP, size, perms, (unsigned long)addr);
    if (new_addr < 0 && new_addr > -100)
        __SET_ERRNO(new_addr);
    else
        __SET_ERRNO(-ESUCCESS);
    return (void*)new_addr;
}

int munmap(void* addr, unsigned long size) {
    __RETURN_WITH_ERRNO((int)system_call(SC_MUNMAP, (unsigned long)addr, size, 0));
}

int mprotect(void* addr, unsigned long size, unsigned long perms) {
    __RETURN_WITH_ERRNO((int)system_call(SC_MPROTECT, (unsigned long)addr, size, perms));
}
