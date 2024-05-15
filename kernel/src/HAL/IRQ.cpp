#include "IRQ.hpp"


#include <Data-structures/LinkedList.hpp>

LinkedList::LockableLinkedList<GlobalInterruptHandler> g_handlers;

#ifdef __x86_64__
#include <arch/x86_64/interrupts/IRQ.hpp>
#include <arch/x86_64/interrupts/isr.hpp>

void arch_HandleGlobalIRQ(x86_64_Interrupt_Registers* regs) {
    g_handlers.lock();
    for (uint64_t i = 0; i < g_handlers.getCount(); i++) {
        GlobalInterruptHandler* handler = g_handlers.get(i);
        if (handler->GetIRQ() == regs->interrupt) {
            g_handlers.unlock();
            handler->HandleInterrupt();
            return;
        }
    }
    g_handlers.unlock();
}

void arch_RegisterHandler(uint8_t IRQ) {
    x86_64_IRQ_RegisterHandler(IRQ, arch_HandleGlobalIRQ);
}

#else
#error Interrupts are not implemented for this architecture!
#endif

InterruptHandlerInfo* RegisterInterruptHandler(uint8_t interrupt_number, InterruptHandler handler, void* data) {
    g_handlers.lock();
    for (uint64_t i = 0; i < g_handlers.getCount(); i++) {
        GlobalInterruptHandler* i_handler = g_handlers.get(i);
        if (i_handler->GetIRQ() == interrupt_number) {
            i_handler->RegisterHandler(handler, data);
            g_handlers.unlock();
            return new InterruptHandlerInfo {
                .handler = handler,
                .interrupt_number = interrupt_number
            };
        }
    }
    GlobalInterruptHandler* global_handler = new GlobalInterruptHandler(interrupt_number);
    global_handler->RegisterHandler(handler, data);
    g_handlers.insert(global_handler);
    g_handlers.unlock();
    return new InterruptHandlerInfo {
        .handler = handler,
        .interrupt_number = interrupt_number
    };
}

void UnregisterInterruptHandler(InterruptHandlerInfo* handler_info) {
    if (handler_info == nullptr)
        return;
    g_handlers.lock();
    for (uint64_t i = 0; i < g_handlers.getCount(); i++) {
        GlobalInterruptHandler* i_handler = g_handlers.get(i);
        arch_RegisterHandler(handler_info->interrupt_number);
        if (i_handler->GetIRQ() == handler_info->interrupt_number) {
            i_handler->UnregisterHandler(handler_info->handler);
            g_handlers.unlock();
            delete handler_info;
            return;
        }
    }
    g_handlers.unlock();
}

GlobalInterruptHandler::GlobalInterruptHandler(uint8_t IRQ) : m_IRQ(IRQ) {
    arch_RegisterHandler(IRQ);
}

void GlobalInterruptHandler::RegisterHandler(InterruptHandler handler, void* data) {
    InterruptHandlerData* i_data = new InterruptHandlerData;
    i_data->handler = handler;
    i_data->data = data;
    m_handlers.lock();
    m_handlers.insert(i_data);
    m_handlers.unlock();
}

void GlobalInterruptHandler::UnregisterHandler(InterruptHandler handler) {
    m_handlers.lock();
    for (uint64_t i = 0; i < m_handlers.getCount(); i++) {
        InterruptHandlerData* data = m_handlers.get(i);
        if (data->handler == handler) {
            m_handlers.remove(i);
            m_handlers.unlock();
            delete data;
            return;
        }
    }
    m_handlers.unlock();
}

void GlobalInterruptHandler::HandleInterrupt() {
    m_handlers.lock();
    for (uint64_t i = 0; i < m_handlers.getCount(); i++) {
        InterruptHandlerData* data = m_handlers.get(i);
        data->handler(data->data);
    }
    m_handlers.unlock();
}

uint8_t GlobalInterruptHandler::GetIRQ() {
    return m_IRQ;
}