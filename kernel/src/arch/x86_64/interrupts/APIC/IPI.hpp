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

#ifndef _X86_64_APIC_IPI_HPP
#define _X86_64_APIC_IPI_HPP

#include <stdint.h>

#include "LocalAPIC.hpp"

enum class x86_64_IPI_DeliveryMode {
    Fixed = 0,
    LowPriority = 1,
    SMI = 2,
    NMI = 4,
    INIT = 5,
    StartUp = 6
};

enum class x86_64_IPI_DestinationShorthand {
    NoShorthand = 0,
    Self = 1,
    AllIncludingSelf = 2,
    AllExcludingSelf = 3
};

void x86_64_SendIPI(x86_64_LocalAPICRegisters* regs, uint8_t vector, x86_64_IPI_DeliveryMode deliveryMode, bool level, bool trigger_mode, x86_64_IPI_DestinationShorthand destShorthand, uint8_t destination);


void x86_64_NMI_IPIHandler(x86_64_Interrupt_Registers*);

#endif /* _X86_64_APIC_IPI_HPP */