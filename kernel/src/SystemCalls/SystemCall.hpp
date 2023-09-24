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

#include <HAL/hal.hpp>

extern "C" {
    
    enum SystemCalls {
        SC_EXIT = 0,
        SC_READ = 1,
        SC_WRITE = 2,
        SC_OPEN = 3,
        SC_CLOSE = 4,
        SC_SEEK = 5
    };

    uint64_t SystemCallHandler(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3, CPU_Registers* regs);
}

void SystemCallInit();

#endif /* _SYSTEM_CALL_HPP */