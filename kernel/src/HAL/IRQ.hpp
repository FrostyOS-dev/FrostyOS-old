#ifndef _HAL_IRQ_HPP
#define _HAL_IRQ_HPP

#include <stdint.h>

#include <Data-structures/LinkedList.hpp>

typedef unsigned int (*InterruptHandler)(void*);

struct InterruptHandlerInfo {
    InterruptHandler handler;
    uint8_t interrupt_number;
};

InterruptHandlerInfo* RegisterInterruptHandler(uint8_t interrupt_number, InterruptHandler handler, void* data);
void UnregisterInterruptHandler(InterruptHandlerInfo* handler_info);

class GlobalInterruptHandler {
public:
    GlobalInterruptHandler(uint8_t IRQ);

    void RegisterHandler(InterruptHandler handler, void* data);

    void UnregisterHandler(InterruptHandler handler);

    void HandleInterrupt();

    uint8_t GetIRQ();

private:
    struct InterruptHandlerData {
        InterruptHandler handler;
        void* data;
    };

    uint8_t m_IRQ;

    LinkedList::LockableLinkedList<InterruptHandlerData> m_handlers;
};

#endif /* _HAL_IRQ_HPP */