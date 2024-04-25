/*
Copyright (©) 2024  Frosty515

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

#include "IPI.hpp"

#include <stdio.h>

#include "../../Processor.hpp"

void x86_64_SendIPI(x86_64_LocalAPICRegisters* regs, uint8_t vector, x86_64_IPI_DeliveryMode deliveryMode, bool level, bool trigger_mode, x86_64_IPI_DestinationShorthand destShorthand, uint8_t destination) {
    uint8_t i_deliveryMode = 0;
    
    switch (deliveryMode) {
        case x86_64_IPI_DeliveryMode::Fixed:
            i_deliveryMode = 0;
            break;
        case x86_64_IPI_DeliveryMode::LowPriority:
            i_deliveryMode = 1;
            break;
        case x86_64_IPI_DeliveryMode::SMI:
            i_deliveryMode = 2;
            break;
        case x86_64_IPI_DeliveryMode::NMI:
            i_deliveryMode = 4;
            break;
        case x86_64_IPI_DeliveryMode::INIT:
            i_deliveryMode = 5;
            break;
        case x86_64_IPI_DeliveryMode::StartUp:
            i_deliveryMode = 6;
            break;
        default:
            return;
    }

    uint8_t shorthand = 0;
    switch (destShorthand) {
        case x86_64_IPI_DestinationShorthand::NoShorthand:
            shorthand = 0;
            break;
        case x86_64_IPI_DestinationShorthand::Self:
            shorthand = 1;
            break;
        case x86_64_IPI_DestinationShorthand::AllIncludingSelf:
            shorthand = 2;
            break;
        case x86_64_IPI_DestinationShorthand::AllExcludingSelf:
            shorthand = 3;
            break;
        default:
            return;
    }
    
    uint32_t ICR0 = *(volatile uint32_t*)(&regs->ICR0);
    uint32_t ICR1 = *(volatile uint32_t*)(&regs->ICR1);

    ICR0 &= 0xFFF32000;
    ICR0 |= vector;
    ICR0 |= (uint32_t)(i_deliveryMode & 0b111) << 8;
    ICR0 |= (level ? 1 : 0) << 14;
    ICR0 |= (trigger_mode ? 1 : 0) << 15;
    ICR0 |= (uint32_t)(shorthand & 0b11) << 18;

    ICR1 &= 0x00FFFFFF;
    ICR1 |= destination << 24;

    //dbgprintf("ICR0 = %#.8x, ICR1 = %#.8x, deliveryMode = %x\n", ICR0, ICR1, i_deliveryMode);

    *(volatile uint32_t*)(&regs->ICR1) = ICR1; // must write to ICR1 first
    *(volatile uint32_t*)(&regs->ICR0) = ICR0;

    while(*(volatile uint32_t*)(&regs->ICR0) & (1 << 12)) { __asm__ volatile ("" ::: "memory"); } // wait for IPI to be sent
}

void x86_64_NMI_IPIHandler(x86_64_Interrupt_Registers* regs) {
    // TODO: add support for more IPI types
    x86_64_StopRequestHandler();
}