#ifndef _HPET_HPP
#define _HPET_HPP

#include <stdint.h>

#include "Device.hpp"

struct HPETGeneralCAPID {
    uint8_t RevisionID;
    uint8_t MaxTimers : 5; // the amount of timers - 1
    uint8_t CounterSize : 1;
    uint8_t Reserved : 1;
    uint8_t LegacyReplacement : 1;
    uint16_t VendorID;
    uint32_t CounterClockPeriod;
} __attribute__((packed));

struct HPETGeneralConfig {
    uint8_t Enable : 1;
    uint8_t LegacyReplacement : 1;
    uint64_t Reserved : 62;
} __attribute__((packed));

struct HPETTimerConfigCAP {
    uint8_t Reserved0 : 1;
    uint8_t IntType : 1;
    uint8_t IntEnable : 1;
    uint8_t Type : 1;
    uint8_t PeriodicSup : 1;
    uint8_t Width : 1; // 0 = 32-bit, 1 = 64-bit
    uint8_t ValueSet : 1;
    uint8_t Reserved1 : 1;
    uint8_t Mode32 : 1;
    uint8_t INTRoute : 5;
    uint8_t FSB : 1;
    uint8_t FSBSup : 1;
    uint16_t Reserved2 : 16;
    uint32_t INTRouteCap : 32; // If bit X is set in this field, it means that this timer can be mapped to IRQX line of I/O APIC.
} __attribute__((packed));

struct HPETTimerRegisters {
    HPETTimerConfigCAP ConfigCAP;
    uint64_t ComparatorValue;
    uint64_t FSBInterruptRoute;
    uint64_t Reserved0;
} __attribute__((packed));

struct HPETRegisters {
    HPETGeneralCAPID GeneralCAPID;
    uint64_t Reserved0;
    HPETGeneralConfig GeneralConfig;
    uint64_t Reserved1;
    uint32_t GeneralINTStatus;
    uint32_t Reserved;
    uint64_t Reserved2[25];
    uint64_t MainCounterValue;
    uint64_t Reserved3;
    HPETTimerRegisters Timer[32];
} __attribute__((packed));

typedef void (*HPETCallback)(void* data);

class HPET : public Device {
public:
    HPET();
    ~HPET();

    void Init(HPETRegisters* regs);

    bool StartTimer(uint64_t femtoSec, HPETCallback callback, void* data);

    void HandleInterrupt(uint8_t interrupt);

    const char* getVendorName() override;
    const char* getDeviceName() override;

private:
    HPETRegisters* m_regs;
    uint16_t m_VendorID;
    uint64_t m_CounterClockPeriod;

    struct Timer {
        bool active;
        bool usable; // if the timer actually exists
        uint64_t femtoSec;
        HPETCallback callback;
        void* data;
        uint8_t IRQ;
    } m_timers[32];
};

extern HPET* g_HPET;

#endif /* _HPET_HPP */