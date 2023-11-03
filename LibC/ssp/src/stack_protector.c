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

#include <stdint.h>
#include <sys/syscall.h>

#define STACK_CHK_GUARD 0x595e9fbd94fda766

uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

__attribute__((noreturn)) void __stack_chk_fail(void) {
#ifndef NDEBUG
    system_call(SC_WRITE, 3, (unsigned long)"Stack smashing detected. terminating...\n", 40);
#endif
    system_call(SC_WRITE, 2, (unsigned long)"Stack smashing detected. terminating...\n", 40);
    system_call(SC_EXIT, 1, 0, 0);
    __builtin_unreachable();
}
