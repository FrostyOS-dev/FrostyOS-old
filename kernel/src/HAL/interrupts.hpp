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

#ifndef _HAL_INTERRUPTS_HPP
#define _HAL_INTERRUPTS_HPP

#include <stdint.h>

#include <Data-structures/LinkedList.hpp>

typedef bool (*InterruptHandler)(void*);

void RegisterInterruptHandler(uint8_t interrupt_number, InterruptHandler handler, void* data);
void UnregisterInterruptHandler(uint8_t interrupt_number, InterruptHandler handler);
bool isValidInterrupt(uint8_t interrupt_number);

class GlobalInterruptHandler {
public:
    GlobalInterruptHandler();

    void RegisterHandler(InterruptHandler handler, void* data);

    void UnregisterHandler(InterruptHandler handler);

    void HandleInterrupt();

private:
    struct InterruptHandlerData {
        InterruptHandler handler;
        void* data;
    };

    LinkedList::SimpleLinkedList<InterruptHandlerData> m_handlers;
};

#endif /* _HAL_INTERRUPTS_HPP */