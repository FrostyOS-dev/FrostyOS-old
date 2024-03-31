#include "IOAPIC.hpp"
#include "LocalAPIC.hpp"

#include <util.h>

#include "../../Processor.hpp"

LinkedList::SimpleLinkedList<x86_64_IOAPIC> g_IOAPICs;

x86_64_IOAPIC::x86_64_IOAPIC(void* baseAddress, uint8_t IRQBase) : m_registers((x86_64_IOAPICRegisters*)baseAddress), m_IRQBase(IRQBase), m_IRQEnd(0), m_INTStart(0) {

}


void x86_64_IOAPIC::WriteRegister(uint32_t reg, uint32_t value) {
    volatile_write32(m_registers->IOREGSEL, reg);
    volatile_write32(m_registers->IOWIN, value);
}

uint32_t x86_64_IOAPIC::ReadRegister(uint32_t reg) {
    volatile_write32(m_registers->IOREGSEL, reg);
    return volatile_read32(m_registers->IOWIN);
}


void x86_64_IOAPIC::SetRedirectionEntry(uint8_t index, x86_64_IOAPIC_RedirectionEntry entry) {
    uint64_t* entryPtr = (uint64_t*)&entry;
    WriteRegister(IOAPIC_REGISTER_REDIRECTION_TABLE + index * 2, *entryPtr & 0xFFFFFFFF);
    WriteRegister(IOAPIC_REGISTER_REDIRECTION_TABLE + index * 2 + 1, *entryPtr >> 32);
}

x86_64_IOAPIC_RedirectionEntry x86_64_IOAPIC::GetRedirectionEntry(uint8_t index) {
    x86_64_IOAPIC_RedirectionEntry entry;
    uint64_t* entryPtr = (uint64_t*)&entry;
    *entryPtr = ReadRegister(IOAPIC_REGISTER_REDIRECTION_TABLE + index * 2);
    *entryPtr |= (uint64_t)ReadRegister(IOAPIC_REGISTER_REDIRECTION_TABLE + index * 2 + 1) << 32;
    return entry;
}

uint8_t x86_64_IOAPIC::GetIRQBase() const {
    return m_IRQBase;
}

uint8_t x86_64_IOAPIC::GetIRQEnd() {
    if (m_IRQEnd == 0) // cache the end for speed
        m_IRQEnd = m_IRQBase + ((ReadRegister(IOAPIC_REGISTER_VERSION) >> 16) & 0xFF) + 1;
    return m_IRQEnd;
}

void x86_64_IOAPIC::SetINTStart(uint8_t INTStart) {
    m_INTStart = INTStart;
    for (uint64_t i = 0; i < (GetIRQEnd() - GetIRQBase()); i++) {
        x86_64_IOAPIC_RedirectionEntry entry = GetRedirectionEntry(i);
        entry.Vector = INTStart + i;
        SetRedirectionEntry(i, entry);
    }
}

uint8_t x86_64_IOAPIC::GetINTStart() const {
    return m_INTStart;
}

x86_64_IOAPIC* x86_64_IOAPIC_GetIOAPICForIRQ(uint8_t IRQ) {
    for (uint64_t i = 0; i < g_IOAPICs.getCount(); i++) {
        x86_64_IOAPIC* ioapic = g_IOAPICs.get(i);
        if (IRQ >= ioapic->GetIRQBase() && IRQ <= ioapic->GetIRQEnd())
            return ioapic;
    }
    return nullptr;
}

void x86_64_IOAPIC_SendEOI() {
    GetCurrentProcessor()->GetLocalAPIC()->SendEOI();
}
