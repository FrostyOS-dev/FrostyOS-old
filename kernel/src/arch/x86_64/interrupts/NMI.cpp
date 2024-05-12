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

#include "NMI.hpp"

#include "APIC/IPI.hpp"

#include "../panic.hpp"
#include "../io.h"

x86_64_NMI_Type x86_64_GetNMIType() {
    /* NMI Priority Order:
    1. Parity
    2. Channel
    3. Watchdog
    4. IPI
    */
    uint8_t SysCntrlB = x86_64_inb(0x61);
    if (SysCntrlB & (1 << 7))
        return x86_64_NMI_Type::Parity;
    if (SysCntrlB & (1 << 6))
        return x86_64_NMI_Type::Channel;

    uint8_t SysCntrlA = x86_64_inb(0x92);
    if (SysCntrlA & (1 << 6))
        return x86_64_NMI_Type::Watchdog;

    return x86_64_NMI_Type::IPI;
}

void x86_64_NMI_Handler(x86_64_Interrupt_Registers* regs) {
    x86_64_NMI_Type type = x86_64_GetNMIType();
    switch (type) {
        case x86_64_NMI_Type::IPI:
            x86_64_NMI_IPIHandler(regs);
            break;
        case x86_64_NMI_Type::Watchdog:
            x86_64_Panic("NMI: Watchdog timer", regs, true);
            break;
        case x86_64_NMI_Type::Channel:
            x86_64_Panic("NMI: Channel check", regs, true);
            break;
        case x86_64_NMI_Type::Parity:
            x86_64_Panic("NMI: Parity check", regs, true);
            break;
    }
}

void x86_64_NMIInit() {
    x86_64_ISR_RegisterHandler(2, x86_64_NMI_Handler);
}