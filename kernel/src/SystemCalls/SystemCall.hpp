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

#ifndef _SYSTEM_CALL_HPP
#define _SYSTEM_CALL_HPP

#include <stdint.h>

typedef uint64_t (*SystemCallHandler_t)(uint64_t, uint64_t, uint64_t);

#define SYSTEM_CALL_COUNT 64

extern "C" {
    
    extern const uint64_t g_syscall_count;
    extern SystemCallHandler_t g_syscall_handlers[SYSTEM_CALL_COUNT];

}

void SystemCallInit();

#endif /* _SYSTEM_CALL_HPP */