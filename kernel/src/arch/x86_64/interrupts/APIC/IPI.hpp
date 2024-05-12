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

#ifndef _X86_64_APIC_IPI_HPP
#define _X86_64_APIC_IPI_HPP

#include <stdint.h>

#include "LocalAPIC.hpp"

enum class x86_64_IPI_DeliveryMode {
    Fixed = 0,
    LowPriority = 1,
    SMI = 2,
    NMI = 4,
    INIT = 5,
    StartUp = 6
};

enum class x86_64_IPI_DestinationShorthand {
    NoShorthand = 0,
    Self = 1,
    AllIncludingSelf = 2,
    AllExcludingSelf = 3
};

enum class x86_64_IPI_Type {
    Stop = 0,
    TLBShootdown = 1,
    NextThread = 2
};

struct x86_64_IPI {
    x86_64_IPI_Type type;
    uint64_t data;
    struct Flags {
        bool wait : 1;
        bool done : 1;
    } flags;

    x86_64_IPI* previous;
    x86_64_IPI* next;
};

struct x86_64_IPI_TLBShootdown {
    uint64_t address;
    uint64_t length;
};

class x86_64_IPI_List {
public:
    x86_64_IPI_List();
    ~x86_64_IPI_List();

    void PushBack(x86_64_IPI* IPI);
    void PushFront(x86_64_IPI* IPI);

    x86_64_IPI* PopBack();
    x86_64_IPI* PopFront();

    uint64_t GetCount() const;

    void Lock() const;
    void Unlock() const;

private:
    x86_64_IPI* m_start;
    x86_64_IPI* m_end;
    uint64_t m_count;

    mutable spinlock_t m_lock;
};

void x86_64_SendIPI(x86_64_LocalAPICRegisters* regs, uint8_t vector, x86_64_IPI_DeliveryMode deliveryMode, bool level, bool trigger_mode, x86_64_IPI_DestinationShorthand destShorthand, uint8_t destination);


void x86_64_NMI_IPIHandler(x86_64_Interrupt_Registers*);

void x86_64_IssueIPI(x86_64_IPI_DestinationShorthand destShorthand, uint8_t destination, x86_64_IPI_Type type, uint64_t data = 0, bool wait = false);

#endif /* _X86_64_APIC_IPI_HPP */