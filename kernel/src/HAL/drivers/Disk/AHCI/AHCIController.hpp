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

#ifndef _AHCI_CONTROLLER_HPP
#define _AHCI_CONTROLLER_HPP

#include <stdint.h>

#include <HAL/drivers/PCIDevice.hpp>

#include "AHCIPort.hpp"

namespace AHCI {

    /* PCI data structures */

    struct AHCIIdentifiers {
        uint16_t VendorID;
        uint16_t DeviceID;
    } __attribute__((packed));

    struct AHCICommandRegister {
        uint8_t IOSE : 1; // I/O Space Enable
        uint8_t MSE : 1; // Memory Space Enable
        uint8_t BME : 1; // Bus Master Enable
        uint8_t SCE : 1; // Special Cycle Enable (reserved)
        uint8_t MWIE : 1; // Memory Write and Invalidate Enable
        uint8_t VPS : 1; // VGA Palette Snoop
        uint8_t PEE : 1; // Parity Error Response Enable
        uint8_t WCC : 1; // Wait Cycle Control (reserved)
        uint8_t SEE : 1; // SERR# Enable
        uint8_t FBE : 1; // Fast Back-to-Back Enable
        uint8_t ID : 1; // Interrupt Disable
        uint8_t Reserved : 5; // reserved
    } __attribute__((packed));

    struct AHCIDeviceStatus {
        uint8_t Reserved0 : 3; // reserved
        uint8_t IS : 1; // Interrupt Status
        uint8_t CL : 1; // Capabilities List
        uint8_t C66 : 1; // 66 MHz Capable
        uint8_t Reserved1 : 1; // reserved
        uint8_t FBC : 1; // Fast Back-to-Back Capable
        uint8_t DPD : 1; // Master Data Parity Error Detected
        uint8_t DEVT : 2; // DEVSEL Timing
        uint8_t STA : 1; // Signaled Target Abort
        uint8_t RMA : 1; // Received Master Abort
        uint8_t SSE : 1; // Signaled System Error
        uint8_t DP : 1; // Detected Parity Error 
    } __attribute__((packed));

    struct AHCIClassCode {
        uint8_t PI; // Programming Interface
        uint8_t SC; // Sub-Class Code
        uint8_t BCC; // Base Class Code
    } __attribute__((packed));

    struct AHCIHeaderType {
        uint8_t HL : 7; // Header Layout
        uint8_t MF : 1; // Multi-Function Device
    } __attribute__((packed));

    struct AHCIBIST {
        uint8_t CC : 4; // Completion Code
        uint8_t Reserved : 2; // reserved
        uint8_t SB : 1; // Start BIST
        uint8_t BC : 1; // BIST Capable
    } __attribute__((packed));

    struct AHCIBaseAddress {
        uint8_t RTE : 1; // Resource Type Indicator
        uint8_t TP : 2; // Type
        uint8_t PF : 1; // Prefetchable
        uint16_t Reserved : 9; // reserved
        uint32_t BA : 19; // Base Address
    } __attribute__((packed));

    struct AHCISubSystemIdentifiers {
        uint16_t SubSystemVendorID;
        uint16_t SubSystemID;
    } __attribute__((packed));

    struct AHCIInterruptInformation {
        uint8_t ILINE; // Interrupt Line
        uint8_t IPIN; // Interrupt Pin
    } __attribute__((packed));

    struct AHCIHeader {
        AHCIIdentifiers ID; // Identifiers
        AHCICommandRegister CMD; // Command Register
        AHCIDeviceStatus STS; // Status
        uint8_t RID; // Revision ID
        AHCIClassCode CC; // Class Codes
        uint8_t CLS; // Cache Line Size
        uint8_t MLT; // Master Latency Timer
        AHCIHeaderType HT; // Header Type
        AHCIBIST BIST; // Built-In Self Test
        AHCIBaseAddress BARS[5]; // Other Base Address Registers
        AHCIBaseAddress ABAR; // AHCI Base Address
        AHCISubSystemIdentifiers SS; // Sub-System Identifiers
        uint32_t EROM; // Expansion ROM Base Address
        uint8_t CP; // Capabilities Pointer
        AHCIInterruptInformation INTR; // Interrupt Information
        uint8_t MGNT; // Minimum Grant
        uint8_t MLAT; // Maximum Latency
    } __attribute__((packed));


    /* HBA data structures */

    struct AHCIHBACapabilities {
        uint8_t NP : 5; // Number of Ports
        uint8_t SXS : 1; // Supports External SATA
        uint8_t EMS : 1; // Enclosure Management Supported
        uint8_t CCCS : 1; // Command Completion Coalescing Supported
        uint8_t NCS : 5; // Number of Command Slots
        uint8_t PSC : 1; // Partial State Capable
        uint8_t SSC : 1; // Slumber State Capable
        uint8_t PMD : 1; // PIO Multiple DRQ Block
        uint8_t FBSS : 1; // FIS-Based Switching Supported
        uint8_t SPM : 1; // Supports Port Multiplier
        uint8_t SAM : 1; // Supports AHCI Mode Only
        uint8_t Reserved0 : 1; // reserved
        uint8_t ISS : 4; // Interface Speed Support
        uint8_t SCLO : 1; // Synchronous Cold
        uint8_t SAL : 1; // Supports Activity LED
        uint8_t SALP : 1; // Supports Aggressive Link Power Management
        uint8_t SSS : 1; // Supports Staggered Spin-up
        uint8_t SMPS : 1; // Supports Mechanical Presence Switch
        uint8_t SSNTF : 1; // Supports SNotification Register
        uint8_t SNCQ : 1; // Supports Native Command Queuing
        uint8_t S64A : 1; // Supports 64-bit Addressing
    } __attribute__((packed));

    struct AHCIGlobalHBAControl {
        uint8_t HR : 1; // HBA Reset
        uint8_t IE : 1; // Interrupt Enable
        uint8_t MRSM : 1; // MSI Revert to Single Message
        uint32_t Reserved : 28; // reserved
        uint8_t AE : 1; // AHCI Enable
    } __attribute__((packed));

    struct AHCIVersion {
        uint16_t Minor;
        uint16_t Major;
    } __attribute__((packed));

    struct AHCICommandCompletionCoalescingControl {
        uint8_t EN : 1; // Enable
        uint8_t Reserved0 : 2; // reserved
        uint8_t INT : 5; // Interrupt
        uint8_t CC : 8; // Command Completions
        uint16_t TV : 16; // Time Value
    } __attribute__((packed));

    struct AHCIEnclosureManagementLocation {
        uint16_t SZ; // Buffer Size
        uint16_t OFST; // Offset
    } __attribute__((packed));

    struct AHCIEnclosureManagementControl {
        uint8_t STS_MR : 1; // Message Received
        uint8_t Reserved0 : 7; // reserved
        uint8_t CTL_TM : 1; // Transmit Message
        uint8_t CTL_RST : 1; // Reset
        uint8_t Reserved1 : 6; // reserved
        uint8_t SUPP_LED : 1; // LED Message Types
        uint8_t SUPP_SAFTE : 1; // SAF-TE Enclosure Management Messages
        uint8_t SUPP_SES2 : 1; // SES-2 Enclosure Management Messages
        uint8_t SUPP_SGPIO : 1; // SGPIO Enclosure Management Messages
        uint8_t Reserved2 : 4; // reserved
        uint8_t ATTR_SMB : 1; // Single Message Buffer
        uint8_t ATTR_XMT : 1; // Transmit Only
        uint8_t ATTR_ALHD : 1; // Activity LED Hardware Driven
        uint8_t ATTR_PM : 1; // Port Multiplier Supported
        uint8_t Reserved3 : 4; // reserved
    } __attribute__((packed));

    struct AHCIHBACapabilitiesExtended {
        uint8_t BOH : 1; // BIOS/OS Handoff
        uint8_t NVMHCI : 1; // NVMHCI Present
        uint8_t APST : 1; // Automatic Partial to Slumber Transitions
        uint8_t SDS : 1; // Supports Device Sleep
        uint8_t SADM : 1; // Supports Aggressive Device Sleep Management
        uint8_t DESO : 1; // DevSleep Entrance from Slumber Only
        uint32_t Reserved : 26; // reserved
    } __attribute__((packed));

    struct AHCIBIOSOSHandoffControlandStatus {
        uint8_t BOS : 1; // BIOS Owned Semaphore
        uint8_t OOS : 1; // OS Owned Semaphore
        uint8_t SOOE : 1; // SMI on OS Ownership Change Enable
        uint8_t OOC : 1; // OS Ownership Change
        uint8_t BB : 1; // BIOS Busy
        uint32_t Reserved : 27; // reserved
    } __attribute__((packed));

    struct AHCIGenericHostControl {
        AHCIHBACapabilities CAP; // Host Capabilities
        AHCIGlobalHBAControl GHC; // Global HBA Control
        uint32_t IS; // Interrupt Status
        uint32_t PI; // Ports Implemented
        AHCIVersion VS; // Version
        AHCICommandCompletionCoalescingControl CCC_CTL; // Command Completion Coalescing Control
        uint32_t CCC_PORTS; // Command Completion Coalescing Ports
        AHCIEnclosureManagementLocation EM_LOC; // Enclosure Management Location
        AHCIEnclosureManagementControl EM_CTL; // Enclosure Management Control
        AHCIHBACapabilitiesExtended CAP2; // Host Capabilities Extended
        AHCIBIOSOSHandoffControlandStatus BOHC; // BIOS/OS Handoff Control and Status
    } __attribute__((packed));

    struct AHCIHBARegisters {
        AHCIGenericHostControl GHC; // Generic Host Control
        uint8_t Reserved0[0x60 - 0x2C]; // reserved
        uint8_t NVMHCI[0xA0 - 0x60]; // NVMHCI
        uint8_t VendorSpecific[0x100 - 0xA0]; // Vendor specific
        AHCIPortRegisters Ports[32]; // Ports
    } __attribute__((packed));

    

    class AHCIController : public PCIDevice {
    public:
        AHCIController();
        ~AHCIController();

        void InitPCIDevice(PCI::Header0* device) override;

        AHCIPortRegisters* GetPortRegisters(uint32_t port_number);

        AHCIHBACapabilities* GetCapabilities();
        AHCIHBACapabilitiesExtended* GetCapabilitiesExtended();

        bool HandleInterrupt();

        const char* getDeviceClass() const override;
        const char* getDeviceSubClass() const override;
        const char* getDeviceProgramInterface() const override;

        const char* getVendorName() override;
        const char* getDeviceName() override;

    private:
        AHCIHBARegisters* m_regs;
        AHCIPort* m_ports[32];
    };

}

#endif /* _AHCI_CONTROLLER_HPP */