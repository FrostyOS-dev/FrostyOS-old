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

#ifndef _AHCI_PORT_HPP
#define _AHCI_PORT_HPP

#include <stdint.h>

namespace AHCI {

    struct AHCIPortCommandListBaseAddress {
        uint16_t Reserved0 : 10; // reserved
        uint32_t CLB : 22; // Command List Base Address
    } __attribute__((packed));

    struct AHCIPortFISBaseAddress {
        uint8_t Reserved0; // reserved
        uint32_t FB : 24; // FIS Base Address
    } __attribute__((packed));

    struct AHCIPortInterruptStatus {
        uint8_t DHRS : 1; // Device to Host Register FIS Interrupt
        uint8_t PSS : 1; // PIO Setup FIS Interrupt
        uint8_t DSS : 1; // DMA Setup FIS Interrupt
        uint8_t SDBS : 1; // Set Device Bits FIS Interrupt
        uint8_t UFS : 1; // Unknown FIS Interrupt
        uint8_t DPS : 1; // Descriptor Processed
        uint8_t PCS : 1; // Port Connect Change Status
        uint8_t DMPS : 1; // Device Mechanical Presence Status
        uint16_t Reserved0 : 14; // reserved
        uint8_t PRCS : 1; // PhyRdy Change Status
        uint8_t IPMS : 1; // Incorrect Port Multiplier Status
        uint8_t OFS : 1; // Overflow Status
        uint8_t Reserved1 : 1;
        uint8_t INFS : 1; // Interface Non-Fatal Error Status
        uint8_t IFS : 1; // Interface Fatal Error Status
        uint8_t HBDS : 1; // Host Bus Data Error Status
        uint8_t HBFS : 1; // Host Bus Fatal Error Status
        uint8_t TFES : 1; // Task File Error Status
        uint8_t CPDS : 1; // Cold Port Detect Status
    } __attribute__((packed));

    struct AHCIPortInterruptEnable {
        uint8_t DHRE : 1; // Device to Host Register FIS Interrupt Enable
        uint8_t PSE : 1; // PIO Setup FIS Interrupt Enable
        uint8_t DSE : 1; // DMA Setup FIS Interrupt Enable
        uint8_t SDBE : 1; // Set Device Bits FIS Interrupt Enable
        uint8_t UFE : 1; // Unknown FIS Interrupt Enable
        uint8_t DPE : 1; // Descriptor Processed Interrupt Enable
        uint8_t PCE : 1; // Port Connect Change Status Enable
        uint8_t DMPE : 1; // Device Mechanical Presence Enable
        uint16_t Reserved0 : 14; // reserved
        uint8_t PRCE : 1; // PhyRdy Change Enable
        uint8_t IPME : 1; // Incorrect Port Multiplier Enable
        uint8_t OFE : 1; // Overflow Enable
        uint8_t Reserved1 : 1;
        uint8_t INFE : 1; // Interface Non-Fatal Error Enable
        uint8_t IFE : 1; // Interface Fatal Error Enable
        uint8_t HBDE : 1; // Host Bus Data Error Enable
        uint8_t HBFE : 1; // Host Bus Fatal Error Enable
        uint8_t TFEE : 1; // Task File Error Enable
        uint8_t CPDE : 1; // Cold Presence Detect Enable
    } __attribute__((packed));

    struct AHCIPortCommandAndStatus {
        uint8_t ST : 1; // Start
        uint8_t SUD : 1; // Spin-Up Device
        uint8_t POD : 1; // Power On Device
        uint8_t CLO : 1; // Command List Override
        uint8_t FRE : 1; // FIS Receive Enable
        uint8_t Reserved0 : 3; // reserved
        uint8_t CCS : 1; // Command and Status
        uint8_t MPSS : 1; // Mechanical Presence Switch State
        uint8_t FR : 1; // FIS Receive Running
        uint8_t CR : 1; // Command List Running
        uint8_t CPS : 1; // Cold Presence State
        uint8_t PMA : 1; // Port Multiplier Attached
        uint8_t HPCP : 1; // Hot Plug Capable Port
        uint8_t MPSP : 1; // Mechanical Presence Switch Attached to Port
        uint8_t CPD : 1; // Cold Presence Detection
        uint8_t ESP : 1; // External SATA Port
        uint8_t FBSCP : 1; // FIS-Based Switching Capable Port
        uint8_t APSTE : 1; // Aggressive Partial to Slumber Transitions Enabled
        uint8_t ATAPI : 1; // Device is ATAPI
        uint8_t DLAE : 1; // Drive LED on ATAPI Enable
        uint8_t ALPE : 1; // Aggressive Link Power Management Enable
        uint8_t ASP : 1; // Aggressive Slumber Partial
        uint8_t ICC : 4; // Interface Communication Control
    } __attribute__((packed));

    struct AHCIPortTaskFileData {
        uint8_t STS_ERR : 1; // Error
        uint8_t STS_CMDSpecific0 : 2; // reserved
        uint8_t STS_DRQ : 1; // Data Request
        uint8_t STS_CMDSpecific1 : 3; // reserved
        uint8_t STS_BSY : 1; // Busy
        uint8_t ERR; // Error register
        uint16_t Reserved; // reserved
    } __attribute__((packed));

    struct AHCIPortSerialATAStatus {
        uint8_t DET : 4; // Device Detection
        uint8_t SPD : 4; // Current Interface Speed
        uint8_t IPM : 4; // Interface Power Management
        uint32_t Reserved0 : 20; // reserved
    } __attribute__((packed));

    struct AHCIPortSerialATAControl {
        uint8_t DET : 4; // Device Detection
        uint8_t SPD : 4; // Desired Interface Speed
        uint8_t IPM : 4; // Interface Power Management
        uint8_t SPM : 4; // Select Power Management (not used by AHCI)
        uint8_t PMP : 4; // Port Multiplier Port (not used by AHCI)
        uint16_t Reserved0 : 12; // reserved
    } __attribute__((packed));

    struct AHCIPortSerialATAError {
        uint16_t ERR; // Error
        uint16_t DIAG; // Diagnostic
    } __attribute__((packed));

    struct AHCIPortSerialATANotification {
        uint16_t PMN; // PM Notify
        uint16_t Reserved; // reserved
    } __attribute__((packed));

    struct AHCIPortFISBasedSwitchingControl {
        uint8_t EN : 1; // Enable
        uint8_t DEC : 1; // Device Error Clear
        uint8_t SDE : 1; // Single Device Error
        uint8_t Reserved0 : 5; // reserved
        uint8_t DEV : 4; // Device To Issue
        uint8_t ADO : 4; // Active Device Optimization
        uint8_t DWE : 1; // Device With Error
        uint16_t Reserved1 : 12; // reserved
    } __attribute__((packed));

    struct AHCIPortDeviceSleep {
        uint8_t ADSE : 1; // Aggressive Device Sleep Enable
        uint8_t DSP : 1; // Device Sleep Present
        uint8_t DETO : 8; // Device Sleep Exit Timeout
        uint8_t MDAT : 5; // Minimum Device Sleep Assertion Time
        uint8_t DITO : 8; // Device Sleep Idle Timeout
        uint8_t DM : 4; // DITO Multiplier
        uint8_t Reserved0 : 3; // reserved
    } __attribute__((packed));

    struct AHCIPortRegisters {
        AHCIPortCommandListBaseAddress CLB; // Command List Base Address
        uint32_t CLBU; // Command List Base Address Upper 32-Bits
        AHCIPortFISBaseAddress FB; // FIS Base Address
        uint32_t FBU; // FIS Base Address Upper 32-Bits
        AHCIPortInterruptStatus IS; // Interrupt Status
        AHCIPortInterruptEnable IE; // Interrupt Enable
        AHCIPortCommandAndStatus CMD; // Command and Status
        uint32_t Reserved0; // reserved
        AHCIPortTaskFileData TFD; // Task File Data
        uint32_t SIG; // Signature
        AHCIPortSerialATAStatus SSTS; // Serial ATA Status
        AHCIPortSerialATAControl SCTL; // Serial ATA Control
        AHCIPortSerialATAError SERR; // Serial ATA Error
        uint32_t SACT; // Serial ATA Active
        uint32_t CI; // Command Issue
        AHCIPortSerialATANotification SNTF; // Serial ATA Notification
        AHCIPortFISBasedSwitchingControl FBS; // FIS-Based Switching Control
        AHCIPortDeviceSleep DEVSLP; // Device Sleep
        uint8_t Reserved1[0x70 - 0x48]; // reserved
        uint8_t VendorSpecific[0x80 - 0x70]; // Vendor Specific
    } __attribute__((packed));

struct AHCICommandHeader {
    struct DW0 {
        uint8_t CFL : 5; // Command FIS Length
        uint8_t A : 1; // ATAPI
        uint8_t W : 1; // Write
        uint8_t P : 1; // Prefetchable
        uint8_t R : 1; // Reset
        uint8_t B : 1; // BIST
        uint8_t C : 1; // Clear Busy upon R_OK
        uint8_t Rsvd0 : 1; // reserved
        uint8_t PMP : 4; // Port Multiplier Port
        uint16_t PRDTL : 16; // Physical Region Descriptor Table Length
    } __attribute__((packed)) DescriptorInformation;
    uint32_t CommandStatus; // bytes transferred so far
    struct DW2 {
        uint8_t Reserved : 7; // reserved
        uint32_t CTBA : 25; // Command Table Base Address
    } __attribute__((packed)) CommandTableBaseAddress;
    uint32_t CommandTableBaseAddressUpper;
    uint32_t Reserved[4]; // reserved
} __attribute__((packed));

    struct AHCICommandList {
        AHCICommandHeader Header[32];
    } __attribute__((packed));

    enum FISTypes {
        FISTypeRegH2D = 0x27,
        FISTypeRegD2H = 0x34,
        FISTypeDMAActivate = 0x39,
        FISTypeDMASetup = 0x41,
        FISTypeData = 0x46,
        FISTypeBIST = 0x58,
        FISTypePIOSetup = 0x5F,
        FISTypeDevBits = 0xA1
    };

    struct FIS_RegH2D {
        uint8_t FISType; // FIS Type
        uint8_t PMPort : 4; // Port Multiplier Port
        uint8_t Reserved0 : 3; // reserved
        uint8_t C : 1; // Command
        uint8_t Command; // Command Register
        uint8_t FeatureLow; // Feature Register, 7:0
        uint8_t LBA0; // LBA Low Register, 7:0
        uint8_t LBA1; // LBA Mid Register, 15:8
        uint8_t LBA2; // LBA High Register, 23:16
        uint8_t Device; // Device Register
        uint8_t LBA3; // LBA Register, 31:24
        uint8_t LBA4; // LBA Register, 39:32
        uint8_t LBA5; // LBA Register, 47:40
        uint8_t FeatureHigh; // Feature Register, 15:8
        uint8_t CountLow; // Count Register, 7:0
        uint8_t CountHigh; // Count Register, 15:8
        uint8_t ICC; // Isochronous Command Completion
        uint8_t Control; // Control Register
        uint8_t Auxiliary[4]; // reserved
    } __attribute__((packed));

    struct FIS_RegD2H {
        uint8_t FISType; // FIS Type
        uint8_t PMPort : 4; // Port Multiplier Port
        uint8_t Reserved0 : 2; // reserved
        uint8_t I : 1; // Interrupt
        uint8_t Reserved1 : 1; // reserved
        uint8_t Status; // Status Register
        uint8_t Error; // Error Register
        uint8_t LBA0; // LBA Low Register, 7:0
        uint8_t LBA1; // LBA Mid Register, 15:8
        uint8_t LBA2; // LBA High Register, 23:16
        uint8_t Device; // Device Register
        uint8_t LBA3; // LBA Register, 31:24
        uint8_t LBA4; // LBA Register, 39:32
        uint8_t LBA5; // LBA Register, 47:40
        uint8_t Reserved2; // reserved
        uint8_t CountLow; // Count Register, 7:0
        uint8_t CountHigh; // Count Register, 15:8
        uint8_t Reserved3[6]; // reserved
    } __attribute__((packed));

    struct FIS_DevBitsD2H {
        uint8_t FISType; // FIS Type
        uint8_t PMPort : 4; // Port Multiplier Port
        uint8_t Reserved0 : 2; // reserved
        uint8_t I : 1; // Interrupt
        uint8_t Notify : 1; // reserved
        uint8_t StatusLow : 3; // Status Low
        uint8_t Reserved1 : 1; // reserved
        uint8_t StatusHigh : 3; // Status High
        uint8_t Reserved2 : 1; // reserved
        uint8_t Error; // Error Register
        uint8_t ProtocolSpecific[4]; // Protocol Specific
    } __attribute__((packed));

    struct FIS_DMAActivateD2H {
        uint8_t FISType; // FIS Type
        uint8_t PMPort : 4; // Port Multiplier Port
        uint8_t Reserved0 : 4; // reserved
        uint8_t Reserved1[2]; // reserved
    } __attribute__((packed));

    struct FIS_DMASetup {
        uint8_t FISType; // FIS Type
        uint8_t PMPort : 4; // Port Multiplier Port
        uint8_t Reserved0 : 1; // reserved
        uint8_t D : 1; // Direction
        uint8_t I : 1; // interrupt
        uint8_t A : 1; // Auto-Activate
        uint8_t Reserved1[2]; // reserved
        uint32_t DMABufferIDLow; // DMA Buffer Identifier Low
        uint32_t DMABufferIDHigh; // DMA Buffer Identifier High
        uint32_t Reserved2; // reserved
        uint32_t DMABufferOffset; // DMA Buffer Offset
        uint32_t DMATransfer; // DMA Transfer Count
        uint32_t Reserved3; // reserved
    } __attribute__((packed));

    struct FIS_BISTActivate {
        uint8_t FISType; // FIS Type
        uint8_t PMPort : 4; // Port Multiplier Port
        uint8_t Reserved0 : 4; // reserved
        struct PD {
            uint8_t V : 1; // Vendor Specific
            uint8_t R : 1; // Reserved
            uint8_t P : 1; // Primitive bit
            uint8_t F : 1; // Far end Analog
            uint8_t L : 1; // Far End Retimed Loopback
            uint8_t S : 1; // Bypass Scrambling
            uint8_t A : 1; // ALIGNp Bypass
            uint8_t T : 1; // Far end transmit only mode

        } __attribute__((packed)) PatternDefinition; // Pattern Definition
        uint8_t Reserved1; // reserved
        uint8_t Data1[4];
        uint8_t Data2[4];
    } __attribute__((packed));
    
    struct FIS_PIOSetup {
        uint8_t FISType; // FIS Type
        uint8_t PMPort : 4; // Port Multiplier Port
        uint8_t Reserved0 : 1; // reserved
        uint8_t D : 1; // Data Transfer Direction
        uint8_t I : 1; // Interrupt
        uint8_t Reserved1 : 1; // reserved
        uint8_t Status; // Status
        uint8_t Error; // Error
        uint8_t LBA0; // LBA Low Register, 7:0
        uint8_t LBA1; // LBA Mid Register, 15:8
        uint8_t LBA2; // LBA High Register, 23:16
        uint8_t Device; // Device Register
        uint8_t LBA3; // LBA Register, 31:24
        uint8_t LBA4; // LBA Register, 39:32
        uint8_t LBA5; // LBA Register, 47:40
        uint8_t Reserved2; // reserved
        uint8_t CountLow; // Count Register, 7:0
        uint8_t CountHigh; // Count Register, 15:8
        uint8_t Reserved3; // reserved
        uint8_t E_Status; // E_Status
        uint16_t TransferCount; // Transfer Count
        uint8_t Reserved4[2]; // reserved
    } __attribute__((packed));

    struct FIS_DataHeader {
        uint8_t FISType; // FIS Type
        uint8_t PMPort : 4; // Port Multiplier Port
        uint8_t Reserved0 : 4; // reserved
        uint8_t Reserved1[2]; // reserved
    } __attribute__((packed));

    struct FIS_Unknown {
        uint8_t Reserved[64];
    } __attribute__((packed));

    struct AHCIReceivedFIS {
        FIS_DMASetup DSFIS;
        uint8_t Reserved0[4];
        FIS_PIOSetup PSFIS;
        uint8_t Reserved1[12];
        FIS_RegD2H RFIS;
        uint8_t Reserved2[4];
        FIS_DevBitsD2H SDBFIS;
        FIS_Unknown UFIS;
        uint8_t Reserved3[96];
    } __attribute__((packed));

    struct AHCICommandTableHeader {
        FIS_Unknown CFIS;
        uint8_t ACMD[16];
        uint8_t Reserved[48];
    } __attribute__((packed));

    struct AHCIPRDTEntry {
        uint8_t Reserved0 : 1;
        uint32_t DataBaseAddress : 31;
        uint32_t DataBaseAddressUpper;
        uint32_t Reserved1;
        uint32_t ByteCount : 22; // Byte Count. '0' based, 4M max, Lowest bit must be 1
        uint32_t Reserved2 : 9; // reserved
        uint32_t InterruptOnCompletion : 1; // Interrupt On Completion
    } __attribute__((packed));

    enum AHCICommands {
        ATA_IDENTIFY_DEVICE = 0xEC
    };

    class AHCIController;

    class AHCIPort {
    public:
        AHCIPort(AHCIController* controller, uint32_t port_number);
        ~AHCIPort();

        void Init();

        void IssueCommandList(AHCICommandHeader** command_list, uint8_t command_table_offset, uint8_t command_table_entries);
        void IssueCommand(AHCICommandHeader* command, uint8_t command_table_offset, bool wait_for_completion = false);

        bool HandleInterrupt();

        void* getCommandList();
        void* getReceivedFIS();

        void* getCommandListPhys();
        void* getReceivedFISPhys();

        uint32_t getPortNumber() const;
        AHCIController* getController() const;
        AHCIPortRegisters* getRegisters() const;

        AHCICommandHeader* GetCommandSlot();
        
    private:

        AHCIController* m_controller;
        uint32_t m_port_number;
        AHCIPortRegisters* m_regs;
        AHCICommandList* m_command_list;
        uint32_t m_command_list_usage;
        AHCIReceivedFIS* m_received_fis;
        void* m_command_list_phys;
        void* m_received_fis_phys;

        void* m_device;
    };

}

#endif /* _AHCI_PORT_HPP */