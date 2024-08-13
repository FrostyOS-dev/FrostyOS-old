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

#include "../../Processor.hpp"

#include "../../Memory/PagingUtil.hpp"

#include "../../Scheduling/taskutil.hpp"

#include <stdio.h>
#include <spinlock.h>

#include <Memory/kmalloc.hpp>

#include <Scheduling/Scheduler.hpp>

x86_64_IPI_List::x86_64_IPI_List() : m_start(nullptr), m_end(nullptr), m_count(0), m_lock(0) {

}

x86_64_IPI_List::~x86_64_IPI_List() {

}

void x86_64_IPI_List::PushBack(x86_64_IPI* IPI) {
    if (m_end != nullptr)
        m_end->next = IPI;
    IPI->previous = m_end;
    m_end = IPI;
    if (m_start == nullptr)
        m_start = IPI;
    m_count++;
}

void x86_64_IPI_List::PushFront(x86_64_IPI* IPI) {
    IPI->next = m_start;
    if (m_start != nullptr)
        m_start->previous = IPI;
    m_start = IPI;
    if (m_end == nullptr)
        m_end = IPI;
    m_count++;
}

x86_64_IPI* x86_64_IPI_List::PopBack() {
    if (m_count == 0)
        return nullptr;
    if (m_count == 1) {
        x86_64_IPI* IPI = m_end;
        m_start = nullptr;
        m_end = nullptr;
        m_count = 0;
        return IPI;
    }
    x86_64_IPI* IPI = m_end;
    m_end = m_end->previous;
    m_end->next = nullptr;
    IPI->previous = nullptr;
    m_count--;
    return IPI;
}

x86_64_IPI* x86_64_IPI_List::PopFront() {
    if (m_count == 0)
        return nullptr;
    if (m_count == 1) {
        x86_64_IPI* IPI = m_start;
        m_start = nullptr;
        m_end = nullptr;
        m_count = 0;
        return IPI;
    }
    x86_64_IPI* IPI = m_start;
    m_start = IPI->next;
    m_start->previous = nullptr;
    IPI->next = nullptr;
    m_count--;
    return IPI;
}

uint64_t x86_64_IPI_List::GetCount() const {
    return m_count;
}

void x86_64_IPI_List::Lock() const {
    spinlock_acquire(&m_lock);
}

void x86_64_IPI_List::Unlock() const {
    spinlock_release(&m_lock);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"

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

    *(volatile uint32_t*)(&regs->ICR1) = ICR1; // must write to ICR1 first
    *(volatile uint32_t*)(&regs->ICR0) = ICR0;

    while(*(volatile uint32_t*)(&regs->ICR0) & (1 << 12)) { __asm__ volatile ("" ::: "memory"); } // wait for IPI to be sent
}

#pragma GCC diagnostic pop

void x86_64_NMI_IPIHandler(x86_64_Interrupt_Registers* regs) {
    Processor* processor = GetCurrentProcessor();
    x86_64_IPI_List& IPIList = processor->GetIPIList();
    IPIList.Lock();
    while (IPIList.GetCount() > 0) {
        x86_64_IPI* IPI = IPIList.PopFront();
        switch (IPI->type) {
        case x86_64_IPI_Type::Stop: // this path won't return, so unlock the list immediately
            if (IPI->flags.wait)
                IPI->flags.done = true;
            else
                kfree_vmm(IPI);
            IPIList.Unlock();
            processor->StopThis();
            break;
        case x86_64_IPI_Type::TLBShootdown: {
            x86_64_IPI_TLBShootdown* data = (x86_64_IPI_TLBShootdown*)IPI->data;
            x86_64_InvalidatePages(data->address, data->length);
            break;
        }
        case x86_64_IPI_Type::NextThread:
            x86_64_SaveIRegistersToThread(Scheduling::Scheduler::GetCurrent(), regs);
            Scheduling::Scheduler::Next(regs);
            break;
        }
        if (IPI->flags.wait)
            IPI->flags.done = true;
        else
            kfree_vmm(IPI);
    }
    IPIList.Unlock();
}

void x86_64_IssueIPI(x86_64_IPI_DestinationShorthand destShorthand, uint8_t destination, x86_64_IPI_Type type, uint64_t data, bool wait) {
    x86_64_IPI IPI = {
        .type = type,
        .data = data,
        .flags = {
            .wait = wait,
            .done = false
        },
        .previous = nullptr,
        .next = nullptr
    };
    size_t list_len = 0;
    switch (destShorthand) {
    case x86_64_IPI_DestinationShorthand::NoShorthand:
    case x86_64_IPI_DestinationShorthand::Self:
        list_len = 1;
        break;
    case x86_64_IPI_DestinationShorthand::AllIncludingSelf:
        list_len = Scheduling::Scheduler::GetProcessorCount();
        break;
    case x86_64_IPI_DestinationShorthand::AllExcludingSelf:
        list_len = Scheduling::Scheduler::GetProcessorCount() - 1;
        break;
    }
    x86_64_IPI** IPIs = (x86_64_IPI**)kcalloc_vmm(list_len, sizeof(x86_64_IPI*));

    switch (destShorthand) {
    case x86_64_IPI_DestinationShorthand::NoShorthand: {
        x86_64_IPI_List& IPIList = Scheduling::Scheduler::GetProcessor(destination)->GetIPIList();
        x86_64_IPI* i_IPI = (x86_64_IPI*)kcalloc_vmm(1, sizeof(x86_64_IPI));
        *i_IPI = IPI;
        IPIs[0] = i_IPI; 
        IPIList.Lock();
        IPIList.PushBack(i_IPI);
        IPIList.Unlock();
        break;
    }
    case x86_64_IPI_DestinationShorthand::Self: {
        x86_64_IPI_List& IPIList = GetCurrentProcessor()->GetIPIList();
        x86_64_IPI* i_IPI = (x86_64_IPI*)kcalloc_vmm(1, sizeof(x86_64_IPI));
        *i_IPI = IPI;
        IPIs[0] = i_IPI;
        IPIList.Lock();
        IPIList.PushBack(i_IPI);
        IPIList.Unlock();
        break;
    }
    case x86_64_IPI_DestinationShorthand::AllIncludingSelf: {
        struct Data {
            x86_64_IPI& IPI;
            x86_64_IPI** IPIs;
            uint64_t i;
        } data = {IPI, IPIs, 0};
        Scheduling::Scheduler::EnumerateProcessors([](Scheduling::Scheduler::ProcessorInfo* info, void* data) {
            Data* i_data = (Data*)data;
            x86_64_IPI_List& IPIList = info->processor->GetIPIList();
            x86_64_IPI* i_IPI = (x86_64_IPI*)kcalloc_vmm(1, sizeof(x86_64_IPI));
            *i_IPI = i_data->IPI;
            i_data->IPIs[i_data->i] = i_IPI;
            IPIList.Lock();
            IPIList.PushBack(i_IPI);
            IPIList.Unlock();
            i_data->i++;
        }, &data);
        break;
    }
    case x86_64_IPI_DestinationShorthand::AllExcludingSelf: {
        Scheduling::Scheduler::ProcessorInfo* info = GetCurrentProcessorInfo();
        struct Data {
            Scheduling::Scheduler::ProcessorInfo* info;
            x86_64_IPI& IPI;
            x86_64_IPI** IPIs;
            uint64_t i;
        } data = {info, IPI, IPIs, 0};
        Scheduling::Scheduler::EnumerateProcessors([](Scheduling::Scheduler::ProcessorInfo* info, void* data) {
            Data* i_data = (Data*)data;
            if (info->id != i_data->info->id) {
                x86_64_IPI_List& IPIList = info->processor->GetIPIList();
                x86_64_IPI* i_IPI = (x86_64_IPI*)kcalloc_vmm(1, sizeof(x86_64_IPI));
                *i_IPI = i_data->IPI;
                i_data->IPIs[i_data->i] = i_IPI;
                IPIList.Lock();
                IPIList.PushBack(i_IPI);
                IPIList.Unlock();
                i_data->i++;
            }
        }, &data);
        break;
    }
    }

    x86_64_SendIPI(GetCurrentProcessor()->GetLocalAPIC()->GetRegisters(), 0, x86_64_IPI_DeliveryMode::NMI, false, false, destShorthand, destination);

    // If the wait flag is set, the IPI struct won't be deleted on completion.

    if (wait) {
        for (size_t i = 0; i < list_len; i++) {
            while (!IPIs[i]->flags.done) { __asm__ volatile ("" ::: "memory"); }
            kfree_vmm(IPIs[i]);
        }
    }
    kfree_vmm(IPIs);
}