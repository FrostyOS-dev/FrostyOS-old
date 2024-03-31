#ifndef _X86_64_IOAPIC_HPP
#define _X86_64_IOAPIC_HPP

#include <stdint.h>

#include <Data-structures/LinkedList.hpp>

enum x86_64_IOAPICRegister_Offsets {
    IOAPIC_REGISTER_ID = 0x00,
    IOAPIC_REGISTER_VERSION = 0x01,
    IOAPIC_REGISTER_ARBITRATION_PRIORITY = 0x02,
    IOAPIC_REGISTER_REDIRECTION_TABLE = 0x10,
};

struct x86_64_IOAPIC_RedirectionEntry {
    uint64_t Vector : 8;
    uint64_t DeliveryMode : 3;
    uint64_t DestinationMode : 1;
    uint64_t DeliveryStatus : 1;
    uint64_t PinPolarity : 1;
    uint64_t RemoteIRR : 1;
    uint64_t TriggerMode : 1;
    uint64_t Mask : 1;
    uint64_t Reserved : 39;
    uint64_t Destination : 8;
} __attribute__((packed));

struct x86_64_IOAPICRegisters {
#define IOAPIC_REGISTER(name) uint32_t name; uint32_t _align_##name[3]
    IOAPIC_REGISTER(IOREGSEL);
    IOAPIC_REGISTER(IOWIN);
#undef IOAPIC_REGISTER
} __attribute__((packed));

class x86_64_IOAPIC {
public:
    x86_64_IOAPIC(void* baseAddress, uint8_t IRQBase);

    void WriteRegister(uint32_t reg, uint32_t value);
    uint32_t ReadRegister(uint32_t reg);

    void SetRedirectionEntry(uint8_t index, x86_64_IOAPIC_RedirectionEntry entry);
    x86_64_IOAPIC_RedirectionEntry GetRedirectionEntry(uint8_t index);

    uint8_t GetIRQBase() const;
    uint8_t GetIRQEnd();

    void SetINTStart(uint8_t INTStart);

    uint8_t GetINTStart() const;


private:
    x86_64_IOAPICRegisters* m_registers;
    uint8_t m_IRQBase;
    uint8_t m_IRQEnd;
    uint8_t m_INTStart;
};

x86_64_IOAPIC* x86_64_IOAPIC_GetIOAPICForIRQ(uint8_t IRQ);


void x86_64_IOAPIC_SendEOI();

extern LinkedList::SimpleLinkedList<x86_64_IOAPIC> g_IOAPICs;

#endif /* _X86_64_IOAPIC_HPP */