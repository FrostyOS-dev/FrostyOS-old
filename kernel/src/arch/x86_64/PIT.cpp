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

#include "PIT.hpp"
#include "io.h"

#include "interrupts/IRQ.hpp"

#include "interrupts/APIC/IOAPIC.hpp"

uint64_t g_Divisor = 0;
uint64_t g_Frequency = 0;
uint64_t g_ticks = 0;

#define DATA_0  0x40
#define DATA_1  0x41
#define DATA_2  0x42
#define COMMAND 0x43

#define BASE_FREQUENCY 1193182

void x86_64_PIT_Handler(x86_64_Interrupt_Registers* iregs) {
    g_ticks += PIT_MS_PER_TICK;
}

void x86_64_PIT_Init() {
    x86_64_outb(COMMAND, 0b00110110 /* Channel 0, lobyte/hibyte, Mode 3 (Square wave generator), 16-bit binary*/);

    x86_64_outb(DATA_0, g_Divisor & 0xFF);
    x86_64_outb(DATA_0, (g_Divisor >> 8) & 0xFF);

    x86_64_IRQ_ReserveIRQ(2);
    x86_64_IRQ_RegisterHandler(2, (x86_64_IRQHandler_t)x86_64_PIT_Handler);
    x86_64_IOAPIC* ioapic = x86_64_IOAPIC_GetIOAPICForIRQ(2);
    x86_64_IOAPIC_RedirectionEntry entry = ioapic->GetRedirectionEntry(2 - ioapic->GetIRQBase());
    entry.Mask = 0;
    ioapic->SetRedirectionEntry(2 - ioapic->GetIRQBase(), entry);

    g_Divisor = 0;
    g_Frequency = 0;
    g_ticks = 0;
}

void x86_64_PIT_SetDivisor(uint64_t div) {
    g_Divisor = div;
    g_Frequency = BASE_FREQUENCY / div;
    x86_64_outb(DATA_0, g_Divisor & 0xFF);
    x86_64_outb(DATA_0, (g_Divisor >> 8) & 0xFF);
}

uint64_t x86_64_PIT_GetTicks() {
    return g_ticks;
}

void x86_64_PIT_SetTicks(uint64_t t) {
    g_ticks = t;
}
