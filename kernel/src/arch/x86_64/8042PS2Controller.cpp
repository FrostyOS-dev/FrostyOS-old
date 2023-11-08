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

#include "8042PS2Controller.hpp"

#include "io.h"

#include "interrupts/IRQ.hpp"
#include "interrupts/pic.hpp"

#define PS2_DATA_PORT 0x60
#define PS2_STATUS_PORT 0x64
#define PS2_COMMAND_PORT 0x64

x86_64_8042_Status_Register x86_64_8042_ReadStatusRegister() {
    uint8_t status = x86_64_inb(PS2_STATUS_PORT);
    x86_64_8042_Status_Register* status_reg = (x86_64_8042_Status_Register*)&status;
    return *status_reg;
}

uint8_t x86_64_8042_ReadStatusRegister_Raw() {
    return x86_64_inb(PS2_STATUS_PORT);
}

void x86_64_8042_WriteCommand(uint8_t command) {
    x86_64_outb(PS2_COMMAND_PORT, command);
}

uint8_t x86_64_8042_ReadData() {
    return x86_64_inb(PS2_DATA_PORT);
}

void x86_64_8042_WriteData(uint8_t data) {
    x86_64_outb(PS2_DATA_PORT, data);
}

struct {
    x86_64_8042_IRQHandler_t handler;
    void* data;
} g_8042_IRQHandlers[2];

void x86_64_8042_IRQHandler0(x86_64_Interrupt_Registers*) {
    g_8042_IRQHandlers[0].handler(g_8042_IRQHandlers[0].data);
}

void x86_64_8042_IRQHandler1(x86_64_Interrupt_Registers*) {
    g_8042_IRQHandlers[1].handler(g_8042_IRQHandlers[1].data);
}

void x86_64_8042_RegisterIRQHandler(x86_64_8042_IRQHandler_t handler, void* data, bool channel) {
    g_8042_IRQHandlers[channel ? 1 : 0] = {handler, data};
    if (channel) {
        x86_64_IRQ_RegisterHandler(12, x86_64_8042_IRQHandler1);
        x86_64_PIC_Unmask(12);
    }
    else {
        x86_64_IRQ_RegisterHandler(1, x86_64_8042_IRQHandler0);
        x86_64_PIC_Unmask(1);
    }
}
