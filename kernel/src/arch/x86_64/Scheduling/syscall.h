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

#ifndef _X86_64_SYSCALL_H
#define _X86_64_SYSCALL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


bool x86_64_IsSystemCallSupported();

uint64_t x86_64_HandleSystemCall();

bool x86_64_EnableSystemCalls(uint16_t kernel_code_segment, uint16_t user_code_segment, uint64_t (*handler)(void));


#ifdef __cplusplus
}
#endif

#endif /* _X86_64_SYSCALL_H */