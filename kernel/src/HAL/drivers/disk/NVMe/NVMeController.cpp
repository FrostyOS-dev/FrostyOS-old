#include "NVMeController.hpp"

#include <Memory/PagingUtil.hpp>
#include <Memory/PageManager.hpp>
#include <Memory/kmalloc.hpp>

#include <assert.h>
#include <util.h>

namespace NVMe {    

    struct IdentifyController {
        uint16_t VID; // PCI Vendor ID
        uint16_t SSVID; // PCI Subsystem Vendor ID
        char SN[20]; // Serial Number
        char MN[40]; // Model Number
        char FR[8]; // Firmware Revision
        uint8_t RAB; // Recommend Arbitration Burst
        uint8_t IEEE[3]; // IEEE Organisation Unique Identifier
        uint8_t CMIC; // Controller Multi-Path I/O and Namespace Sharing Capabilities
        uint8_t MDTS; // Maximum Data Transfer Size
        uint16_t CNTLID; // Controller ID
        uint32_t VER; // Version
        uint32_t RTD3R; // RTD3 Resume Latency
        uint32_t RTD3E; // RTD3 Entry Latency
        uint32_t OAES; // Optional Asynchronous Events Supported
        uint32_t CTRATT; // Controller Attributes
        uint16_t RRLS; // Read Recovery Levels Supported
        uint8_t Reserved0[9];
        uint8_t CNTRLTYPE; // Controller Type
        uint64_t FGUID[2]; // FRU Globally Unique Identifier
        uint16_t CRDT[3]; // Command Retry Delay Time 1-3
        uint8_t Reserved1[106];
        uint8_t ResNMI[13]; // Reserved for the NVMe Management Interface
        uint8_t NVMSR; // NVM Subsystem Report
        uint8_t VWCI; // VPD Write Cycle Information
        uint8_t MEC; // Management Endpoint Capabilites
        uint16_t OACS; // Option Admin Command Support
        uint8_t ACL; // Abort Command Line
        uint8_t AERL; // Asynchronous Event Request Limit
        uint8_t FRMW; // Firmware Updates
        uint8_t LPA; // Log Page Attributes
        uint8_t ELPE; // Error Log Page Entries
        uint8_t NPSS; // Number of Power States Support
        uint8_t AVSCC; // Admin Vendor Specific Command Configuration
        uint8_t APSTA; // Autonomous Power State Transition Attributes
        uint16_t WCTEMP; // Warning Composite Temperature Threshold
        uint16_t CCTEMP; // Critical Composite Temperature Threshold
        uint16_t MTFA; // Maximum Time for Firmware Activation
        uint32_t HMPRE; // Host Memory Buffer Preferred Size
        uint32_t HMMIN; // Host Memory Buffer Minimum Size
        uint64_t TNVMCAP[2]; // Total NVM Capacity
        uint64_t UNVMCAP[2]; // Unallocated NVM Capacity
        uint32_t RPMBS; // Replay Protected Memory Block Support
        uint16_t EDSTT; // Extended Device Self-test Time
        uint8_t DSTO; // Device Self-test Options
        uint8_t FWUG; // Firmware Update Granularity
        uint16_t KAS; // Keep Alive Support
        uint16_t HCTMA; // Host Controlled Thermal Management Attributes
        uint16_t MNTMT; // Minimum Thermal Management Temperature
        uint16_t MXTMT; // Maximum Thermal Management Temperature
        uint32_t SANICAP; // Sanitize Capabilities
        uint32_t HMMINDS; // Host Memory Buffer Minimum Descriptor Entry Size
        uint16_t HMMAXD; // Host Memory Maximum Descriptors Entries
        uint16_t NSETIDMAX; // NVM Set Identifier Maximum
        uint16_t ENDGIDMAX; // Endurance Group Identifier Maximum
        uint8_t ANATT; // ANA Transition Time
        uint8_t ANACAP; // Asymmetric Namespace Access Capabilites
        uint32_t ANAGRPNMAX; // ANA Group Identifier Maximum
        uint32_t NANAGRID; // Number of ANA Group Identifiers
        uint32_t PELS; // Persistent Even Log Size
        uint16_t DomainID;
        uint8_t Reserved2[10];
        uint64_t MEGCAP[2]; // Max Endurance Group Capacity
        uint8_t Reserved3[128];
        uint8_t SQES; // Submission Queue Entry Size
        uint8_t CQES; // Completion Queue Entry Size
        uint16_t MAXCMD; // Maximum Outstanding Commands
        uint32_t NN; // Number of Namespaces
        uint16_t ONCS; // Optional NVM Command Support
        uint16_t FUSES; // Fused Operation Support
        uint8_t FNA; // Format NVM Attributes
        uint8_t VWCl; // Volatile Write Cache
        uint16_t AWUN; // Atomic Write Unit Normal
        uint16_t AWUPF; // Atmoic Write Unit Power Fail
        uint8_t ICSVSCC; // I/O Command Set Vendor Specific Command Configuration
        uint8_t NWPC; // Namespace Write Protection Capabilites
        uint16_t ACWU; // Atomic Compare & Write Unit
        uint16_t CDFS; // Copy Descriptor Formats Supported
        uint32_t SGLS; // SGL Support
        uint32_t MNAN; // Maximum Number of Allow Namespaces
        uint64_t MAXDNA[2]; // Maximum Domain Namespace Attachments
        uint32_t MAXCNA; // Maximum I/O Controller Namespace Attachments
        uint8_t Reserved4[204];
        char SUBNQN[256]; // NVM Subsystem NVMe Qualified Name
        uint8_t Reserved5[768];
        uint32_t IOCCQSZ; // I/O Queue Command Capsule Supported Size
        uint32_t IORCSZ; // I/O Queue Response Capsule Supported Size
        uint16_t ICDOFF; // In Capsule Data Offset
        uint8_t FCATT; // Fabrics Controller Attributes
        uint8_t MSDBD; // Maximum SGL Data Block Descriptors
        uint16_t OFCS; // Optional Fabric Commands Support
        uint8_t Reserved6[242];
        uint64_t PSD[4][32]; // Power State Descriptor 0-31
        uint8_t VendorSpecific[1024];
    } __attribute__((packed));

    NVMeController::NVMeController() : m_BAR0(nullptr), m_IRQ(0), m_admin_queue(nullptr), m_ControllerInfo(nullptr), m_NSIDList(nullptr) {

    }

    NVMeController::~NVMeController() {

    }

    SubmissionQueueEntry entry;

    void NVMeController::InitPCIDevice(PCI::Header0* device) {
        if (p_device != nullptr)
            return;
        if (device == nullptr)
            return;
        p_device = device;
        PCI::MemSpaceBaseAddressRegister mem_bar = *(PCI::MemSpaceBaseAddressRegister*)&(p_device->BAR0);
        assert(mem_bar.always0 == 0);
        assert(mem_bar.type == 0x02);
        m_BAR0 = to_HHDM((void*)((mem_bar.AlignedBaseAddress << 4) + ((uint64_t)(p_device->BAR1) << 32)));
        MapPage((void*)((mem_bar.AlignedBaseAddress << 4) | ((uint64_t)(p_device->BAR1) << 32)), m_BAR0, 0x8000003); // Present, Read/Write, Execute Disable
        MapPage((void*)(((mem_bar.AlignedBaseAddress << 4) | ((uint64_t)(p_device->BAR1) << 32)) + 0x1000), (void*)((uint64_t)m_BAR0 + 0x1000), 0x8000003); // Present, Read/Write, Execute Disable
        m_IRQ = p_device->INTLine;
        assert(m_IRQ != 0xFF);
        NVMe::BAR0* BAR0 = (NVMe::BAR0*)m_BAR0;
        assert(BAR0->CAP.MPSMIN == 0);
        uint32_t doorbell_stride = 4 << BAR0->CAP.DSTRD; // get the stride before the controller is disabled
        BAR0->CC.Enable = 0; // Disable the controller
        do {
            assert(BAR0->CSTS.CFS == 0);
        } while (BAR0->CSTS.Ready != 0); // Wait until the controller is stopped
        BAR0->AQA = 0x003F003F; // 64 entries for Admin submission Queue and Admin completion queue
        m_admin_queue = new NVMeAdminQueue;
        m_admin_queue->Create((void*)((uint64_t)m_BAR0 + 0x1000), doorbell_stride, 64, 0);
        void* ACQ = m_admin_queue->GetCompletionQueue();
        assert(ACQ != nullptr);
        void* ASQ = m_admin_queue->GetSubmissionQueue();
        assert(ASQ != nullptr);
        BAR0->ACQ = (uint64_t)get_physaddr(ACQ);
        BAR0->ASQ = (uint64_t)get_physaddr(ASQ);
        BAR0->CC.IOCQES = 4; // 2^4 = 16 (sizeof(CompletionQueueEntry))
        BAR0->CC.IOSQES = 6; // 2^6 = 64 (sizeof(SubmissionQueueEntry))
        BAR0->CC.CSS = 0b000;
        BAR0->CC.MPS = 0;
        BAR0->CC.Enable = 1; // Start the controller
        do {
            assert(BAR0->CSTS.CFS == 0);
        } while (BAR0->CSTS.Ready != 1); // Wait until the controller is running
        NVMeIOQueue* IOQueue = new NVMeIOQueue;
        IOQueue->Create((void*)((uint64_t)m_BAR0 + 0x1000), doorbell_stride, 64, 1);

        fast_memset(&entry, 0, sizeof(SubmissionQueueEntry) / 8);
        entry.command.Opcode = (uint8_t)AdminCommands::CREATE_IO_COMPLETION_QUEUE;
        void* IOCQ = IOQueue->GetCompletionQueue();
        assert(IOCQ != nullptr);
        entry.DataPTR0 = (uint64_t)get_physaddr(IOCQ);
        entry.DWORD10 = 0x003F0001; // 64-entries in queue, ID 1
        entry.DWORD11 = 1; // physically contiguous
        assert(m_admin_queue->SendCommand(&entry));

        fast_memset(&entry, 0, sizeof(SubmissionQueueEntry) / 8);
        entry.command.Opcode = (uint8_t)AdminCommands::CREATE_IO_SUBMISSION_QUEUE;
        void* IOSQ = IOQueue->GetCompletionQueue();
        assert(IOSQ != nullptr);
        entry.DataPTR0 = (uint64_t)get_physaddr(IOSQ);
        entry.DWORD10 = 0x003F0001; // 64 entries in queue, ID 1
        entry.DWORD11 = 0x00010001; // IOCQ ID 1, physically contiguous
        assert(m_admin_queue->SendCommand(&entry));

        fast_memset(&entry, 0, sizeof(SubmissionQueueEntry) / 8);
        entry.command.Opcode = (uint8_t)AdminCommands::IDENTIFY;
        m_ControllerInfo = (IdentifyController*)WorldOS::g_KPM->AllocatePage();
        assert(m_ControllerInfo != nullptr);
        fast_memset(m_ControllerInfo, 0, 4096 / 8);
        entry.DataPTR0 = (uint64_t)get_physaddr(m_ControllerInfo);
        entry.DWORD10 = 1; // The controller
        assert(m_admin_queue->SendCommand(&entry));

        fast_memset(&entry, 0, sizeof(SubmissionQueueEntry) / 8);
        entry.command.Opcode = (uint8_t)AdminCommands::IDENTIFY;
        m_NSIDList = (uint32_t*)WorldOS::g_KPM->AllocatePage();
        assert(m_NSIDList != nullptr);
        fast_memset(m_NSIDList, 0, 4096 / 8);
        entry.DataPTR0 = (uint64_t)get_physaddr(m_NSIDList);
        entry.DWORD10 = 2; // NSID list
        assert(m_admin_queue->SendCommand(&entry));

        uint64_t MaxTransferSize = 0;
        if (m_ControllerInfo->MDTS != 0) {
            MaxTransferSize = UINT64_C(1) << m_ControllerInfo->MDTS;
            MaxTransferSize *= 0x1000 << BAR0->CAP.MPSMIN;
            fprintf(VFS_DEBUG, "[%s(%lp)] INFO: Maximum transfer size in bytes is %lu.\n", __extension__ __PRETTY_FUNCTION__, device, MaxTransferSize);
        }

        uint_fast16_t NSIndex = 0;
        //while (m_NSIDList[NSIndex] != 0) {
            NVMeDisk* disk = new NVMeDisk(1, this, IOQueue, MaxTransferSize);
            m_Disks.insert(disk);
            uint8_t* data = (uint8_t*)kcalloc(disk->GetSectorSize());
            assert(data != nullptr);
            assert(disk->Read(data, 0, 1));
            fprintf(VFS_DEBUG, "\nPrinting disk %lu sector 0...\n\n\n", NSIndex);
            for (uint64_t i = 0; i < disk->GetSectorSize(); i++) {
                fputc(VFS_DEBUG, data[i]);
            }
            fprintf(VFS_DEBUG, "\n\n\n");
            NSIndex++;
        //}
        fprintf(VFS_DEBUG, "NVMe Success!\n");
    }

    IdentifyNamespace* NVMeController::IdentifyDisk(uint32_t ID) const {
        NVMe::SubmissionQueueEntry entry;
        fast_memset(&entry, 0, sizeof(SubmissionQueueEntry) / 8);
        entry.command.Opcode = (uint8_t)AdminCommands::IDENTIFY;
        IdentifyNamespace* NID = (IdentifyNamespace*)WorldOS::g_KPM->AllocatePage();
        assert(NID != nullptr);
        fast_memset(NID, 0, 4096 / 8);
        entry.DataPTR0 = (uint64_t)get_physaddr(NID);
        entry.DWORD10 = 0; // A namespace
        entry.NSID = ID;
        if (m_admin_queue->SendCommand(&entry))
            return NID;
        else {
            WorldOS::g_KPM->FreePage(NID);
            return nullptr;
        }
    }

    const char* NVMeController::getVendorName() const {

    }

    const char* NVMeController::getDeviceName() const {

    }

    const char* NVMeController::getDeviceClass() const {
        return "Mass Storage Controller";
    }

    const char* NVMeController::getDeviceSubClass() const {
        return "Non-Volatile Memory Controller";
    }

    const char* NVMeController::getDeviceProgramInterface() const {
        return "NVM Express";
    }
}