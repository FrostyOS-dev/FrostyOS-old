/*
Copyright (©) 2022-2024  Frosty515

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

#ifndef _X86_64_IRQ_HPP
#define _X86_64_IRQ_HPP

#include "isr.hpp"

#define INVALID_IRQ 0xFF

typedef void (*x86_64_IRQHandler_t)(x86_64_Interrupt_Registers* regs);

/*
0x00-0x1F - reserved for exceptions
0x20-0x2F - reserved for the legacy 8259 PICs
0x30-0xEF - available for I/O APICs
0xF0      - LAPIC timer
0xF1-0xFF - LAPIC/IPI reserved
*/

void x86_64_IRQ_EarlyInit(); // just configure and disable the PIC
void x86_64_IRQ_FullInit(); // get the I/O APIC(s) ready
void x86_64_IRQ_RegisterHandler(const uint8_t irq, x86_64_IRQHandler_t handler);

void x86_64_IRQ_ReserveIRQ(uint8_t irq);
void x86_64_IRQ_UnreserveIRQ(uint8_t irq);
bool x86_64_IRQ_IsIRQReserved(uint8_t irq);
uint8_t x86_64_IRQ_AllocateIRQ();
uint8_t x86_64_IRQ_AllocateIRQ(uint64_t mask, uint64_t shift); // each bit in mask corresponds to an IRQ, shift is the number of bits to shift the mask
void x86_64_IRQ_FreeIRQ(uint8_t irq);

#endif /* _X86_64_IRQ_HPP */