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

    // Returns the value of the main counter in ticks
    uint64_t GetMainCounter() const;
    uint64_t* GetMainCounterAddress() const;

    uint64_t GetClockPeriod() const;


    void HandleInterrupt(uint8_t interrupt);

    const char* getVendorName() const override;
    const char* getDeviceName() const override;

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