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

#include "AHCIPort.hpp"
#include "AHCIController.hpp"
#include "AHCIDisk.hpp"

#include <stdio.h>
#include <util.h>

#include <Memory/PageManager.hpp>

#include <HAL/time.h>

namespace AHCI {

    AHCIPort::AHCIPort(AHCIController* controller, uint32_t port_number) : m_controller(controller), m_port_number(port_number), m_command_list(nullptr) {

    }

    AHCIPort::~AHCIPort() {

    }

    void AHCIPort::Init() {
        // Get the port registers from the controller
        m_regs = (AHCIPortRegisters*)m_controller->GetPortRegisters(m_port_number);

        if (m_regs->SSTS.DET != 3)
            return;

        /* We must do these things in order for this port:
        - Read signature/status of the port to see if it connected to a drive.
        - Send IDENTIFY ATA command to connected drives. Get their sector size and count.
        */
        // Stop the port
        m_regs->CMD.ST = 0;
        __asm__ volatile("" ::: "memory");
        while (m_regs->CMD.CR == 1)
            __asm__ volatile("" ::: "memory");

        m_regs->CMD.FRE = 0;
        __asm__ volatile("" ::: "memory");
        while (m_regs->CMD.FR == 1)
            __asm__ volatile("" ::: "memory");

        // Allocate physical memory for the command list, received FIS, and command tables
        m_command_list = (AHCICommandList*)g_KPM->AllocatePages(DIV_ROUNDUP(sizeof(AHCICommandList), PAGE_SIZE), PagePermissions::READ_WRITE, nullptr, true);
        m_command_list_usage = 0;
        memset(m_command_list, 0, sizeof(AHCICommandList));
        m_command_list_phys = g_KPM->GetPageTable().GetPhysicalAddress(m_command_list);

        for (uint8_t i = 0; i < 32; i++) {
            m_command_tables[i] = (AHCICommandTable*)g_KPM->AllocatePages(DIV_ROUNDUP(sizeof(AHCICommandTable), PAGE_SIZE), PagePermissions::READ_WRITE, nullptr, true);
            memset(m_command_tables[i], 0, sizeof(AHCICommandTable));
            void* command_table_phys = g_KPM->GetPageTable().GetPhysicalAddress(m_command_tables[i]);
            m_command_list->Header[i].CommandTableBaseAddress.CTBA = ((uint64_t)command_table_phys & 0xFFFFFFFF) >> 7;
            m_command_list->Header[i].CommandTableBaseAddressUpper = ((uint64_t)command_table_phys >> 32) & 0xFFFFFFFF;
        }

        // Reset the port
        m_regs->SCTL.DET = 1;
        sleep(1);
        m_regs->SCTL.DET = 0;
        __asm__ volatile("" ::: "memory");
        while (m_regs->SSTS.DET != 3 && m_regs->SSTS.IPM != 1)
            __asm__ volatile("" ::: "memory");

        *((uint32_t*)&(m_regs->SERR)) = 0xFFFFFFFF;

        // Spin up device if supported
        if (m_controller->GetCapabilities()->SSS == 1) {
            dbgprintf("AHCI: Spinning up device on port %d\n", m_port_number);
            m_regs->CMD.SUD = 1;
            __asm__ volatile("" ::: "memory");
        }

        // Set the command list address
        m_regs->CLB.CLB = ((uint64_t)m_command_list_phys & 0xFFFFFFFF) >> 10;
        m_regs->CLBU = ((uint64_t)m_command_list_phys >> 32) & 0xFFFFFFFF;

        m_received_fis = (AHCIReceivedFIS*)g_KPM->AllocatePage();
        memset(m_received_fis, 0, sizeof(AHCIReceivedFIS));
        m_received_fis_phys = g_KPM->GetPageTable().GetPhysicalAddress(m_received_fis);

        // Set the received FIS address
        m_regs->FB.FB = ((uint64_t)m_received_fis_phys & 0xFFFFFFFF) >> 8;
        m_regs->FBU = ((uint64_t)m_received_fis_phys >> 32) & 0xFFFFFFFF;
        

        // Start the port
        __asm__ volatile("" ::: "memory");
        while (m_regs->CMD.CR != 0)
            __asm__ volatile("" ::: "memory");


        m_regs->CMD.FRE = 1;
        m_regs->CMD.ST = 1;

        m_regs->CMD.ICC = 1; // ensure the port is active


        // Enable interrupts
        memset(&(m_regs->IE), 0, sizeof(AHCIPortInterruptEnable));
        m_regs->IE.DHRE = 1;
        m_regs->IE.HBFE = 1;
        m_regs->IE.HBDE = 1;
        m_regs->IE.PSE = 1;

        // read the signature
        uint32_t sig = m_regs->SIG;
        switch (sig) {
        case 0x00000101: {
            dbgprintf("AHCI: SATA drive on port %d\n", m_port_number);
            AHCIDisk* disk = new AHCIDisk(this);
            m_device = disk;
            disk->Init();
            break;
        }
        case 0xEB140101:
            dbgprintf("AHCI: SATAPI drive on port %d\n", m_port_number);
            break;
        case 0xC33C0101:
            dbgprintf("AHCI: Enclosure management bridge on port %d\n", m_port_number);
            break;
        case 0x96690101:
            dbgprintf("AHCI: Port multiplier on port %d\n", m_port_number);
            break;
        }
    }

    void AHCIPort::IssueCommandList(AHCICommandHeader** command_list, uint8_t command_table_offset, uint8_t command_table_entries) {
        assert(command_table_entries > 0);
        assert((command_table_offset + command_table_entries) <= 32);
        for (uint64_t i = 0; i < command_table_entries; i++)
            memcpy(&(m_command_list->Header[command_table_offset + i]), command_list[i], sizeof(AHCICommandHeader));
        m_regs->SACT |= ((1UL << command_table_entries) - 1) << command_table_offset;
        m_regs->CI |= ((1UL << command_table_entries) - 1) << command_table_offset;
    }

    void AHCIPort::IssueCommand(AHCICommandHeader* command, uint8_t, bool wait_for_completion) {
        //assert(command_table_offset < 32);
        //memcpy(&(m_command_list->Header[command_table_offset]), command, sizeof(AHCICommandHeader));
        // Wait for the port to be ready
        __asm__ volatile("" ::: "memory");
        while (m_regs->TFD.STS_BSY == 1 || m_regs->TFD.STS_DRQ == 1)
            __asm__ volatile("" ::: "memory");

        uint8_t command_table_offset = 0;
        for (uint8_t i = 0; i < 32; i++) {
            if (&(m_command_list->Header[i]) == command) {
                command_table_offset = i;
                break;
            }
        }
        
        // Send the command
        //m_regs->SACT |= 1UL << command_table_offset;
        m_regs->CI |= 1UL << command_table_offset;

        if (wait_for_completion) {
            __asm__ volatile("" ::: "memory");
            while ((m_regs->CI & (1UL << command_table_offset)) != 0)
                __asm__ volatile("" ::: "memory");
        }
        // Print interrupt status
        __asm__ volatile ("" ::: "memory");
        while (m_regs->IS.DHRS == 0)
            __asm__ volatile ("" ::: "memory");
        uint32_t* is = ((uint32_t*)&(m_regs->IS));
        dbgprintf("AHCI: Port %d interrupt status: %x\n", m_port_number, *is);

        // print command completion status from m_received_fis.RFIS
        dbgprintf("AHCI: Port %d command completion status: %hx, serr = %x\n", m_port_number, *(uint16_t*)(&(m_regs->TFD)), *(uint32_t*)(&(m_regs->SERR)));
    }

    bool AHCIPort::HandleInterrupt() {
        dbgprintf("AHCI: Port %d interrupt\n", m_port_number);
        return false;
    }

    void* AHCIPort::getCommandList() {
        return m_command_list;
    }

    void* AHCIPort::getReceivedFIS() {
        return m_received_fis;
    }

    void* AHCIPort::getCommandListPhys() {
        return m_command_list_phys;
    }

    void* AHCIPort::getReceivedFISPhys() {
        return m_received_fis_phys;
    }

    uint32_t AHCIPort::getPortNumber() const {
        return m_port_number;
    }

    AHCIController* AHCIPort::getController() const {
        return m_controller;
    }

    AHCIPortRegisters* AHCIPort::getRegisters() const {
        return m_regs;
    }

    AHCICommandHeader* AHCIPort::GetCommandSlot() {
        for (uint8_t i = 0; i < 32; i++) {
            if ((m_command_list_usage & (1UL << i)) == 0) {
                m_command_list_usage |= (1UL << i);
                return &(m_command_list->Header[i]);
            }
        }
    }

    AHCICommandTable* AHCIPort::getCommandTable(AHCICommandHeader* header) {
        for (uint8_t i = 0; i < 32; i++) {
            if (&(m_command_list->Header[i]) == header)
                return m_command_tables[i];
        }
    }

};