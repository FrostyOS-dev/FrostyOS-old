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

#include "AHCIController.hpp"

#include <assert.h>
#include <stdio.h>

#include <Memory/PagingUtil.hpp>

#include <HAL/interrupts.hpp>

namespace AHCI {

    AHCIController::AHCIController() {

    }

    AHCIController::~AHCIController() {

    }

    void AHCIController::InitPCIDevice(PCI::Header0* device) {
        if (device == nullptr)
            return;
        p_device = device;
        PCI::CommandRegister* command_reg = (PCI::CommandRegister*)&(p_device->ch.Command);
        command_reg->BusMaster = 1;
        command_reg->MemorySpace = 1;
        command_reg->IOSpace = 1; // FIXME: verify whether this is correct
        command_reg->InterruptDisable = 0;

        PCI::MemSpaceBaseAddressRegister* base_addr_reg = (PCI::MemSpaceBaseAddressRegister*)&(p_device->BAR5);
        assert(base_addr_reg->always0 == 0);
        assert(base_addr_reg->type == 0);
        void* BAR5 = to_HHDM((void*)((uint64_t)(base_addr_reg->AlignedBaseAddress << 4) & 0xFFFFFFFF));
        m_regs = (AHCIHBARegisters*)BAR5;
        assert(m_regs->GHC.VS.Major == 1); // AHCI 1.x

        // We must perform BIOS/OS handoff if supported
        if (m_regs->GHC.CAP2.BOH == 1) {
            m_regs->GHC.BOHC.OOS = 1;
            while (m_regs->GHC.BOHC.BOS == 1 || m_regs->GHC.BOHC.OOS == 0) {}
            dbgprintf("AHCI: BIOS/OS handoff complete\n");
        }

        // Now we reset the controller
        /*m_regs->GHC.GHC.HR = 1;
        __asm__ volatile ("" ::: "memory");
        while (m_regs->GHC.GHC.HR != 0) {
            __asm__ volatile ("" ::: "memory");
        }*/

        // Now we enable AHCI mode
        m_regs->GHC.GHC.AE = 1;

        // Verify that 64-bit DMA is supported
        assert(m_regs->GHC.CAP.S64A == 1);

        // Enable interrupt handler
        m_regs->GHC.GHC.IE = 1;
        RegisterInterruptHandler(p_device->INTLine, (InterruptHandler)&AHCIController::HandleInterrupt, this);

        // Enumerate through implemented ports
        for (uint32_t i = 0; i < 32; i++) {
            if (m_regs->GHC.PI & (1UL << i)) {
                m_ports[i] = new AHCIPort(this, i);
                m_ports[i]->Init();
            }
        }
    }

    AHCIPortRegisters* AHCIController::GetPortRegisters(uint32_t port_number) {
        assert(port_number < 32);
        return &(m_regs->Ports[port_number]);
    }

    AHCIHBACapabilities* AHCIController::GetCapabilities() {
        return &(m_regs->GHC.CAP);
    }

    AHCIHBACapabilitiesExtended* AHCIController::GetCapabilitiesExtended() {
        return &(m_regs->GHC.CAP2);
    }

    bool AHCIController::HandleInterrupt() {
        dbgprintf("AHCI: Interrupt received\n");
        bool handled = false;
        for (uint32_t i = 0; i < 32; i++) {
            if (m_ports[i] != nullptr)
                handled |= m_ports[i]->HandleInterrupt();
        }
        return handled;
    }

    const char* AHCIController::getDeviceClass() const {
        return "Mass storage controller";
    }

    const char* AHCIController::getDeviceSubClass() const {
        return "SATA controller";
    }

    const char* AHCIController::getDeviceProgramInterface() const {
        return "AHCI 1.0";
    }

    const char* AHCIController::getVendorName() {
        if (p_device == nullptr)
            return "Unknown";
        switch (p_device->ch.VendorID) {
        case 0x8086:
            return "Intel Corporation";
        default:
            return "Unknown";
        }
    }

    const char* AHCIController::getDeviceName() {
        if (p_device == nullptr)
            return "Unknown";
        switch (p_device->ch.VendorID) {
        case 0x8086: // Intel
            switch (p_device->ch.DeviceID) {
            case 0x2922:
                return "82801IR/IO/IH (ICH9R/DO/DH) 6 port SATA Controller [AHCI mode]";
            default:
                return "Unknown";
            }
        default:
            return "Unknown";
        }
    }

}
