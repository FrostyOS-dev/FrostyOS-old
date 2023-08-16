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

#ifndef _WORLDOS_KERNEL_X86_64_IO_h
#define _WORLDOS_KERNEL_X86_64_IO_h

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void x86_64_outb(uint16_t port, uint8_t value);
extern uint8_t x86_64_inb(uint16_t port);

extern void x86_64_EnableInterrupts();
extern void x86_64_DisableInterrupts();

extern void x86_64_iowait();

#ifdef __cplusplus
}
#endif

#endif /* _WORLDOS_KERNEL_X86_64_IO_h */