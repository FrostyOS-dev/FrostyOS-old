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

#ifndef _KERNEL_X86_64_ELF_SYMBOLS_H
#define _KERNEL_X86_64_ELF_SYMBOLS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern uint8_t __text_start;
extern uint8_t __text_end;
extern uint8_t __rodata_start;
extern uint8_t __rodata_end;
extern uint8_t __data_start;
extern uint8_t __data_end;
extern uint8_t __bss_start;
extern uint8_t __bss_end;
extern uint8_t __kernel_end;

extern const void* _text_start_addr;
extern const void* _text_end_addr;
extern const void* _rodata_start_addr;
extern const void* _rodata_end_addr;
extern const void* _data_start_addr;
extern const void* _data_end_addr;
extern const void* _bss_start_addr;
extern const void* _bss_end_addr;
extern const void* _kernel_end_addr;


#ifdef __cplusplus 
}
#endif

#endif /* _KERNEL_X86_64_ELF_SYMBOLS_H */