/*
Copyright (©) 2022-2023  Frosty515

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

#include <arch/x86_64/GDT.hpp>

#include <arch/x86_64/interrupts/IDT.hpp>
#include <arch/x86_64/interrupts/isr.hpp>
#include <arch/x86_64/interrupts/IRQ.hpp>
#include <arch/x86_64/interrupts/pic.hpp>

#include <arch/x86_64/io.h>
#include <arch/x86_64/panic.hpp>

#include <arch/x86_64/Memory/PagingInit.hpp>

#include "drivers/ACPI/RSDP.hpp"
#include "drivers/ACPI/XSDT.hpp"
#include "drivers/ACPI/MCFG.hpp"

#include "drivers/PCI.hpp"

#include "drivers/disk/NVMe/NVMeController.hpp"

#include "time.h"
#include "hal.hpp"

#include <tty/TTY.hpp>

#include <assert.h>
#include <stdio.hpp>
#include <string.h>

#include <Memory/newdelete.hpp>
#include <Memory/PagingUtil.hpp>

namespace WorldOS {

    void HAL_EarlyInit(MemoryMapEntry** MemoryMap, uint64_t MMEntryCount, uint64_t kernel_virtual, uint64_t kernel_physical, uint64_t kernel_size, uint64_t HHDM_start, const FrameBuffer& fb) {
        x86_64_GDTInit();

        x86_64_IDT_Initialize();
        x86_64_ISR_Initialize();
        x86_64_IDT_Load(&idt.idtr);

        x86_64_IRQ_Initialize();

        x86_64_DisableInterrupts();
        HAL_TimeInit();

        x86_64_InitPaging(MemoryMap, MMEntryCount, kernel_virtual, kernel_physical, kernel_size, (uint64_t)(fb.FrameBufferAddress), ((fb.bpp >> 3) * fb.FrameBufferHeight * fb.FrameBufferWidth), HHDM_start);

        x86_64_SetPanicVGADevice(g_CurrentTTY->GetVGADevice());

        x86_64_EnableInterrupts();
    }

    void HAL_Stage2(void* RSDP) {
        assert(InitAndValidateRSDP(RSDP));
        assert(IsXSDTAvailable());
        assert(InitAndValidateXSDT(GetXSDT()));
        bool MCFGFound = false;
        for (uint64_t i = 0; i < getSDTCount(); i++) {
            ACPISDTHeader* header = getOtherSDT(i);
            if (strncmp(header->Signature, "MCFG", 4) == 0 && !MCFGFound) {
                assert(InitAndValidateMCFG(header));
                MCFGFound = true;
            }
        }
        assert(MCFGFound); // it must be found or device detection won't work
        for (uint64_t i = 0; i < GetMCFGEntryCount(); i++) {
            MCFGEntry* entry = GetMCFGEntry(i);
            PCI::EnumerateBuses(to_HHDM((void*)(entry->Address)));
        }
        PCI::Header0* device = PCI::PCIDeviceList::GetPCIDevice(0);
        for (uint64_t i = 1; device != nullptr; i++) {
            if (device->ch.ClassCode == 0x1 && device->ch.SubClass == 0x8 && device->ch.ProgIF == 0x2) { // NVMe
                NVMe::NVMeController* controller = new NVMe::NVMeController;
                fprintf(VFS_DEBUG, "PCI Device: VendorID=%hx DeviceID=%hx Class=\"%s\" SubClass=\"%s\" Program Interface=\"%s\"\n", device->ch.VendorID, device->ch.DeviceID, controller->getDeviceClass(), controller->getDeviceSubClass(), controller->getDeviceProgramInterface());
                controller->InitPCIDevice(device);
            }
            else
                fprintf(VFS_DEBUG, "PCI Device: VendorID=%hx DeviceID=%hx Class=%hhx SubClass=%hhx Program Interface=%hhx\n", device->ch.VendorID, device->ch.DeviceID, device->ch.ClassCode, device->ch.SubClass, device->ch.ProgIF);
            device = PCI::PCIDeviceList::GetPCIDevice(i);
        }
    }

}
