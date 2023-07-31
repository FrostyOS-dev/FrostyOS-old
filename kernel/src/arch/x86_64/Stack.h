/*
Copyright (©) 2022-2023  Frosty515

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

#ifndef _X86_64_KERNEL_STACK_H
#define _X86_64_KERNEL_STACK_H

#ifdef __cplusplus
extern "C" {
#endif

#define KERNEL_STACK_SIZE 65536

extern unsigned char __attribute__((aligned(0x1000))) kernel_stack[KERNEL_STACK_SIZE];

extern unsigned long int kernel_stack_size;

#ifdef __cplusplus
}
#endif

#endif /* _X86_64_KERNEL_STACK_H */