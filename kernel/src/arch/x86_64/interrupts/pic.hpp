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

#ifndef _KERNEL_X86_64_PIC_HPP
#define _KERNEL_X86_64_PIC_HPP

#include <stdint.h>
#include <stddef.h>

void x86_64_PIC_SetMask(uint16_t newMask);
uint16_t x86_64_PIC_GetMask();
void x86_64_PIC_sendEOI(uint8_t irq);
void x86_64_PIC_Disable();
void x86_64_PIC_Mask(uint8_t irq);
void x86_64_PIC_Unmask(uint8_t irq);
void x86_64_PIC_Configure(uint8_t offset_PIC1, uint8_t offset_PIC2, bool autoEOI);
uint16_t x86_64_PIC_ReadIRQRequestRegister();
uint16_t x86_64_PIC_ReadInServiceRegister();
bool x86_64_PIC_Probe();

#endif /* _KERNEL_X86_64_PIC_HPP */