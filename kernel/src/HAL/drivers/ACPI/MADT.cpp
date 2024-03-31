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

#include "MADT.hpp"

#include <Data-structures/LinkedList.hpp>

#include <stdio.h>

#include <Memory/PagingUtil.hpp>

#ifdef __x86_64__
#include <arch/x86_64/interrupts/APIC/LocalAPIC.hpp>
#include <arch/x86_64/interrupts/APIC/IOAPIC.hpp>

#include <arch/x86_64/interrupts/IRQ.hpp>
#endif

#include "../../hal.hpp"

ACPISDTHeader* g_MADT;

bool InitAndValidateMADT(ACPISDTHeader* MADT) {
    if (MADT == nullptr)
        return false;
    if (doChecksum(MADT)) {
        g_MADT = MADT;
        return true;
    }
    return false;
}

void EnumerateMADTEntries() {
    MADTEntriesHeader* entriesHeader = (MADTEntriesHeader*)((uint64_t)g_MADT + sizeof(ACPISDTHeader));
    void* LAPIC_address = (void*)(uint64_t)(entriesHeader->LAPICAddress);

    uint8_t APIC_ID;
#ifdef __x86_64__
    __asm__ volatile ("mov $1, %%eax; cpuid; shr $24, %%ebx;" : "=b"(APIC_ID) : : "eax", "ecx", "edx");
#endif
    dbgprintf("BSP APIC ID = %d\n", APIC_ID);

    LinkedList::SimpleLinkedList<MADT_LocalAPIC> LAPICs;
    LinkedList::SimpleLinkedList<MADT_IOAPIC> IOAPICs;
    LinkedList::SimpleLinkedList<MADT_InterruptSourceOverride> InterruptSourceOverrides;

    MADTEntryHeader* start = (MADTEntryHeader*)((uint64_t)g_MADT + sizeof(ACPISDTHeader) + sizeof(MADTEntriesHeader));
    MADTEntryHeader* end = (MADTEntryHeader*)((uint64_t)g_MADT + g_MADT->Length);
    MADTEntryHeader* entry = start;
    while (entry < end) {
        switch (entry->Type) {
            case 0: {
                MADT_LocalAPIC* localAPIC = (MADT_LocalAPIC*)entry;
                dbgprintf("Local APIC: ACPI Processor ID: %d, APIC ID: %d, Flags: %d\n", localAPIC->ACPIProcessorID, localAPIC->APICID, localAPIC->Flags);
                LAPICs.insert(localAPIC);
                break;
            }
            case 1: {
                MADT_IOAPIC* ioAPIC = (MADT_IOAPIC*)entry;
                dbgprintf("IO APIC: ID: %d, Address: %x, Global System Interrupt Base: %d\n", ioAPIC->IOAPICID, ioAPIC->IOAPICAddress, ioAPIC->GlobalSystemInterruptBase);
                IOAPICs.insert(ioAPIC);
                break;
            }
            case 2: {
                MADT_InterruptSourceOverride* interruptSourceOverride = (MADT_InterruptSourceOverride*)entry;
                dbgprintf("Interrupt Source Override: Bus: %d, Source: %d, Global System Interrupt: %d, Flags: %d\n", interruptSourceOverride->Bus, interruptSourceOverride->Source, interruptSourceOverride->GlobalSystemInterrupt, interruptSourceOverride->Flags);
                break;
            }
            case 3: {
                MADT_IOAPICNMISource* ioAPICNMISource = (MADT_IOAPICNMISource*)entry;
                dbgprintf("IO APIC NMI Source: Flags: %d, Global System Interrupt: %d\n", ioAPICNMISource->Flags, ioAPICNMISource->GlobalSystemInterrupt);
                break;
            }
            case 4: {
                MADT_LocalAPICNMISource* localAPICNMISource = (MADT_LocalAPICNMISource*)entry;
                dbgprintf("Local APIC NMI Source: ACPI Processor ID: %d, Flags: %d, Local APIC LINT: %d\n", localAPICNMISource->ACPIProcessorID, localAPICNMISource->Flags, localAPICNMISource->LocalAPICLint);
                break;
            }
            case 5: {
                MADT_LocalAPICAddressOverride* localAPICAddressOverride = (MADT_LocalAPICAddressOverride*)entry;
                dbgprintf("Local APIC Address Override: Address: %x\n", localAPICAddressOverride->LocalAPICAddress);
                LAPIC_address = (void*)(localAPICAddressOverride->LocalAPICAddress);
                break;
            }
            case 9: {
                MADT_Localx2APIC* localx2APIC = (MADT_Localx2APIC*)entry;
                dbgprintf("Local x2APIC: ACPI Processor ID: %u, Local APIC ID: %u, Flags: %u\n", localx2APIC->ACPIProcessorUID, localx2APIC->x2APICID, localx2APIC->Flags);
                break;
            }
            default:
                dbgprintf("Unknown MADT entry type: %d\n", entry->Type);
                break;
        }
        entry = (MADTEntryHeader*)((uint64_t)entry + entry->Length);
    }

    for (uint64_t i = 0; i < IOAPICs.getCount(); i++) {
        MADT_IOAPIC* ioAPIC = IOAPICs.get(i);
        x86_64_IOAPIC* ioapic = new x86_64_IOAPIC(to_HHDM((void*)(uint64_t)(ioAPIC->IOAPICAddress)), ioAPIC->GlobalSystemInterruptBase);
        for (uint64_t j = ioapic->GetIRQBase(); j < ioapic->GetIRQEnd(); j++) {
            x86_64_IOAPIC_RedirectionEntry entry;
            entry.DeliveryMode = 0;
            entry.DestinationMode = 0;
            entry.DeliveryStatus = 0;
            entry.PinPolarity = 0;
            entry.RemoteIRR = 0;
            entry.TriggerMode = 0;
            entry.Mask = 1;
            entry.Destination = APIC_ID; // always send to the BSP
            ioapic->SetRedirectionEntry(j - ioapic->GetIRQBase(), entry); // we want to mask everything by default
        }
        for (uint64_t j = 0; j < InterruptSourceOverrides.getCount(); j++) {
            MADT_InterruptSourceOverride* interruptSourceOverride = InterruptSourceOverrides.get(j);
            if (interruptSourceOverride->GlobalSystemInterrupt >= ioapic->GetIRQBase() && interruptSourceOverride->GlobalSystemInterrupt < ioapic->GetIRQEnd()) {
                x86_64_IOAPIC_RedirectionEntry entry;
                entry.DeliveryMode = 0;
                entry.DestinationMode = 0;
                entry.DeliveryStatus = 0;
                entry.PinPolarity = (interruptSourceOverride->Flags & 2) >> 1;
                entry.RemoteIRR = 0;
                entry.TriggerMode = (interruptSourceOverride->Flags & 8) >> 3;
                entry.Mask = 1;
                entry.Destination = APIC_ID; // always send to the BSP
                ioapic->SetRedirectionEntry(interruptSourceOverride->GlobalSystemInterrupt - ioapic->GetIRQBase(), entry);
            }
        }
        g_IOAPICs.insert(ioapic);
    }

    x86_64_IRQ_FullInit();

    for (uint64_t j = 0; j < InterruptSourceOverrides.getCount(); j++) {
        MADT_InterruptSourceOverride* interruptSourceOverride = InterruptSourceOverrides.get(j);
        x86_64_IRQ_ReserveIRQ(interruptSourceOverride->GlobalSystemInterrupt);
    }

    for (uint64_t i = 0; i < LAPICs.getCount(); i++) {
        MADT_LocalAPIC* localAPIC = LAPICs.get(i);
        if (localAPIC->APICID != APIC_ID) {
            x86_64_LocalAPIC* lapic = new x86_64_LocalAPIC(to_HHDM(LAPIC_address), false, localAPIC->APICID);
            lapic->StartCPU();
        }
        else {
            x86_64_LocalAPIC* lapic = new x86_64_LocalAPIC(to_HHDM(LAPIC_address), true, localAPIC->APICID);
            g_BSP.SetLocalAPIC(lapic);
            g_BSP.InitialiseLocalAPIC();
        }
    }

}

