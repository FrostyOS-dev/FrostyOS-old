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

#include "interrupts.hpp"

#ifdef __x86_64__
#include <arch/x86_64/interrupts/IRQ.hpp>
#include <arch/x86_64/interrupts/pic.hpp>

GlobalInterruptHandler* g_IRQHandlers[5];

void IRQ9Handler(x86_64_Interrupt_Registers* regs) {
    if (g_IRQHandlers[0] != nullptr)
        g_IRQHandlers[0]->HandleInterrupt();
}

void IRQ10Handler(x86_64_Interrupt_Registers* regs) {
    dbgprintf("IRQ 10. handler = %lp\n", g_IRQHandlers[1]);
    if (g_IRQHandlers[1] != nullptr)
        g_IRQHandlers[1]->HandleInterrupt();
}

void IRQ11Handler(x86_64_Interrupt_Registers* regs) {
    if (g_IRQHandlers[2] != nullptr)
        g_IRQHandlers[2]->HandleInterrupt();
}

void IRQ14Handler(x86_64_Interrupt_Registers* regs) {
    if (g_IRQHandlers[3] != nullptr)
        g_IRQHandlers[3]->HandleInterrupt();
}

void IRQ15Handler(x86_64_Interrupt_Registers* regs) {
    if (g_IRQHandlers[4] != nullptr)
        g_IRQHandlers[4]->HandleInterrupt();
}


void RegisterInterruptHandler(uint8_t interrupt_number, InterruptHandler handler, void* data) {
    dbgprintf("Registering interrupt handler for interrupt %d\n", interrupt_number);
    if (IN_BOUNDS(interrupt_number, 9, 11)) {
        uint8_t index = interrupt_number - 9;
        if (g_IRQHandlers[index] == nullptr) {
            switch (interrupt_number) {
            case 9:
                x86_64_IRQ_RegisterHandler(9, IRQ9Handler);
                break;
            case 10:
                x86_64_IRQ_RegisterHandler(10, IRQ10Handler);
                break;
            case 11:
                x86_64_IRQ_RegisterHandler(11, IRQ11Handler);
                break;
            }
            g_IRQHandlers[index] = new GlobalInterruptHandler();
        }
        g_IRQHandlers[index]->RegisterHandler(handler, data);
        x86_64_PIC_Unmask(interrupt_number);
    }
    else if (IN_BOUNDS(interrupt_number, 14, 15)) {
        uint8_t index = interrupt_number - 14 + 2;
        if (g_IRQHandlers[index] == nullptr) {
            switch (interrupt_number) {
            case 14:
                x86_64_IRQ_RegisterHandler(14, IRQ14Handler);
                break;
            case 15:
                x86_64_IRQ_RegisterHandler(15, IRQ15Handler);
                break;
            }
            g_IRQHandlers[index] = new GlobalInterruptHandler();
        }
        g_IRQHandlers[index]->RegisterHandler(handler, data);
        x86_64_PIC_Unmask(interrupt_number);
    }
}

void UnregisterInterruptHandler(uint8_t interrupt_number, InterruptHandler handler) {
    if (IN_BOUNDS(interrupt_number, 9, 11)) {
        uint8_t index = interrupt_number - 9;
        if (g_IRQHandlers[index] == nullptr)
            return;
        g_IRQHandlers[index]->UnregisterHandler(handler);
    }
    else if (IN_BOUNDS(interrupt_number, 14, 15)) {
        uint8_t index = interrupt_number - 14 + 2;
        if (g_IRQHandlers[index] == nullptr)
            return;
        g_IRQHandlers[index]->UnregisterHandler(handler);
    }
}

bool isValidInterrupt(uint8_t interrupt_number) {
    return IN_BOUNDS(interrupt_number, 9, 11) || IN_BOUNDS(interrupt_number, 14, 15);
}

#else
#error Interrupts are not implemented for this architecture!
#endif

GlobalInterruptHandler::GlobalInterruptHandler() {

}

void GlobalInterruptHandler::RegisterHandler(InterruptHandler handler, void* data) {
    InterruptHandlerData* i_data = new InterruptHandlerData;
    i_data->handler = handler;
    i_data->data = data;
    m_handlers.insert(i_data);
}

void GlobalInterruptHandler::UnregisterHandler(InterruptHandler handler) {
    for (uint64_t i = 0; i < m_handlers.getCount(); i++) {
        InterruptHandlerData* data = m_handlers.get(i);
        if (data->handler == handler) {
            m_handlers.remove(i);
            delete data;
            return;
        }
    }
}

void GlobalInterruptHandler::HandleInterrupt() {
    for (uint64_t i = 0; i < m_handlers.getCount(); i++) {
        InterruptHandlerData* data = m_handlers.get(i);
        if (data->handler(data->data))
            return;
    }
}