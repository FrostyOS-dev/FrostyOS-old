#include "NVMe.hpp"

#include <Memory/PagingUtil.hpp>
#include <Memory/PageManager.hpp>

#include <assert.h>
#include <stdio.hpp>
#include <util.h>

namespace NVMe {

    struct Command {
        uint8_t Opcode;
        uint8_t FusedOperation : 2;
        uint8_t reserved : 4;
        uint8_t PRP_SGL : 2; // PRP or SGL selection. 0 indicates PRPs.
        uint16_t CommandID; // This is put in the completion queue entry.
    } __attribute__((packed));

    struct SubmissionQueueEntry {
        Command command;
        uint32_t NSID; // Namespace identifier. If N/A, set to 0
        uint64_t reserved;
        uint64_t MetadataPTR;
        uint64_t DataPTR0;
        uint64_t DataPTR1;
        uint64_t CommandSpecific0;
        uint64_t CommandSpecific1;
        uint64_t CommandSpecific2;
    } __attribute__((packed));

    struct CompletionQueueEntry {
        uint32_t CommandSpecfic;
        uint32_t Reserved;
        uint16_t SubmissionQueueHeadPTR;
        uint16_t SubmissionQueueID;
        uint16_t CommandID;
        uint8_t Phase : 1; // Toggled when entry written.
        uint16_t Status : 15; // 0 on success
    } __attribute__((packed));

    enum Commands {
        CREATE_IO_SUBMISSION_QUEUE = 0x01,
        CREATE_IO_COMPLETION_QUEUE = 0x05,
        IDENTIFY = 0x06,
        READ = 0x02,
        WRITE = 0x01
    };

    struct ControllerConfiguration {
        uint8_t Enable : 1;
        uint8_t Reserved0 : 3;
        uint8_t CSS : 3; // I/O Command Set Selected
        uint8_t MPS : 4; // Memory Page Size
        uint8_t AMS : 3; // Arbitration Mechanism Selected
        uint8_t SHN : 2; // Shutdown Notification
        uint8_t IOSQES : 4; // I/O Submission Queue Entry Size
        uint8_t IOCQES : 4; // I/O Completion Queue Entry Size
        uint8_t CRIME : 1; // Controller Ready Independent of Media Enable
        uint8_t Reserved1 : 7;
    } __attribute__((packed));

    struct ControllerStatus {
        uint8_t Ready : 1;
        uint8_t CFS : 1; // Controller Fatal Status 
        uint8_t SHST : 2; // Shutdown Status
        uint8_t NSSRO : 1; // NVM Subsystem Reset Occurred
        uint8_t PP : 1; // Processing Paused
        uint8_t ST : 1; // Shutdown Type
        uint8_t Reserved : 25;
    } __attribute__((packed));

    struct ControllerCapabilities {
        uint16_t MQES; // Maximum Queue Entries Supported
        uint8_t CQR : 1; // Contiguous Queues Required
        uint8_t AMS : 2; // Arbitration Mechanism Supported
        uint8_t Reserved0 : 5;
        uint8_t Timeout;
        uint8_t DSTRD : 4; // Doorbell Stride. Specified as (2 ^ (2 + DSTRD)) in bytes
        uint8_t NSSRS : 1; // NVM Subsystem Reset Supported
        uint8_t CSS : 8; // Command Sets Supported (Due to a unusual issue, this field must be a bitfield because it is not on a byte boundary)
        uint8_t BPS : 1; // Boot Partition Support
        uint8_t CPS : 2; // Controller Power Scope
        uint8_t MPSMIN : 4; // Memory Page Size Minimum
        uint8_t MPSMAX : 4; // Memory Page Size Maximum
        uint8_t PMRS : 1; // Persistent Memory Region Supported
        uint8_t CMBS : 1; // Controller Memory Buffer Supported
        uint8_t NSSS : 1; // NVM Subsystem Shutdown Supported
        uint8_t CRMS : 2; // Controller Ready Modes Supported
        uint8_t Reserved1 : 3;
    } __attribute__((packed));

    struct BAR0 {
        ControllerCapabilities CAP; // Controller capabilities.
        uint32_t VS; // Version.
        uint32_t INTMS; // Interrupt mask set.
        uint32_t INTMC; // Interrupt mask clear.
        ControllerConfiguration CC; // Controller configuration.
        uint32_t padding0;
        ControllerStatus CSTS; // Controller status.
        uint32_t padding1;
        uint32_t AQA; // Admin queue attributes.
        uint64_t ASQ; // Admin submission queue.
        uint64_t ACQ; // Admin completion queue.
        uint64_t padding2[0x1F9]; // Get to 4096 bytes
    } __attribute__((packed));

    struct IdentifyController {
        uint16_t VID; // PCI Vendor ID
        uint16_t SSVID; // PCI Subsystem Vendor ID
        char SN[20]; // Serial Number
        char MN[40]; // Model Number
        char FR[8]; // Firmware Revision
        uint8_t RAB; // Recommend Arbitration Burst
        uint16_t IEEE0; // IEEE Organisation Unique Identifier
        uint8_t IEEE1;
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
        uint64_t FGUID0; // FRU Globally Unique Identifier 
        uint64_t FGUID1;
        uint8_t unknown[3968];
    } __attribute__((packed));

}

/* NVMeDisk class */

/* Public Methods */

NVMeDisk::NVMeDisk() : m_BAR0(nullptr), m_IRQ(0), m_ASQ_address(nullptr), m_ACQ_address(nullptr), m_IOSQ_address(nullptr), m_IOCQ_address(nullptr), m_ControllerInfo(nullptr), m_NSIDList(nullptr), m_NSID1Info(nullptr) {

}

NVMeDisk::~NVMeDisk() {

}

void NVMeDisk::InitPCIDevice(PCI::Header0* device) {
    p_device = device;
    PCI::MemSpaceBaseAddressRegister mem_bar = *(PCI::MemSpaceBaseAddressRegister*)&(p_device->BAR0);
    assert(mem_bar.always0 == 0);
    assert(mem_bar.type == 0x2);
    m_BAR0 = to_HHDM((void*)((mem_bar.AlignedBaseAddress << 4) + ((uint64_t)(p_device->BAR1) << 32)));
    MapPage((void*)((mem_bar.AlignedBaseAddress << 4) | ((uint64_t)(p_device->BAR1) << 32)), m_BAR0, 0x8000003); // Read/Write, Present, Execute Disable
    MapPage((void*)((mem_bar.AlignedBaseAddress << 4) | ((uint64_t)(p_device->BAR1) << 32) + 0x1000), (void*)((uint64_t)m_BAR0 + 0x1000), 0x8000003); // Read/Write, Present, Execute Disable
    m_IRQ = p_device->INTLine;
    assert(m_IRQ != 0xFF);
    NVMe::BAR0* BAR0 = (NVMe::BAR0*)m_BAR0;
    BAR0->CC.Enable = 0; // Disable the controller
    do {
        assert(BAR0->CSTS.CFS == 0);
    } while (BAR0->CSTS.Ready != 0); // Wait until the controller is stopped
    BAR0->AQA = 0x003F003F; // 64 entries for both
    m_ACQ_address = WorldOS::g_KPM->AllocatePage();
    assert(m_ACQ_address != nullptr);
    fast_memset(m_ACQ_address, 0, sizeof(NVMe::CompletionQueueEntry) / 8);
    m_ASQ_address = WorldOS::g_KPM->AllocatePage();
    assert(m_ASQ_address != nullptr);
    fast_memset(m_ASQ_address, 0, sizeof(NVMe::SubmissionQueueEntry) / 8);
    BAR0->ACQ = (uint64_t)get_physaddr(m_ACQ_address); // Set Admin Completion Queue address
    BAR0->ASQ = (uint64_t)get_physaddr(m_ASQ_address); // Set Admin Submission Queue address
    BAR0->CC.IOCQES = 4; // 2^4 = 16 (sizeof(NVMe::CompletionQueueEntry))
    BAR0->CC.IOSQES = 6; // 2^6 = 64 (sizeof(NVMe::SubmissionQueueEntry))
    BAR0->CC.CSS = 0b000;
    BAR0->CC.MPS = 0; // 4KiB pages
    BAR0->CC.Enable = 1; // Start the controller
    do {
        assert(BAR0->CSTS.CFS == 0);
    } while (BAR0->CSTS.Ready != 1); // Wait until the controller is running
    NVMe::SubmissionQueueEntry entry;

    fast_memset(&entry, 0, sizeof(NVMe::SubmissionQueueEntry) / 8);
    entry.command.Opcode = NVMe::CREATE_IO_COMPLETION_QUEUE;
    m_IOCQ_address = WorldOS::g_KPM->AllocatePage();
    assert(m_IOCQ_address != nullptr);
    fast_memset(m_IOCQ_address, 0, sizeof(NVMe::CompletionQueueEntry) * 64 / 8);
    entry.DataPTR0 = (uint64_t)get_physaddr(m_IOCQ_address); // Set I/O Completion Queue address
    entry.CommandSpecific0 = ((0x003F0001) << 32) | (0x00000001); // 64 entries in queue, ID 1, physically contiguous
    AdminCommand(&entry);

    fast_memset(&entry, 0, sizeof(NVMe::SubmissionQueueEntry) / 8);
    entry.command.Opcode = NVMe::CREATE_IO_SUBMISSION_QUEUE;
    m_IOSQ_address = WorldOS::g_KPM->AllocatePage();
    assert(m_IOSQ_address != nullptr);
    fast_memset(m_IOSQ_address, 0, sizeof(NVMe::SubmissionQueueEntry) * 64 / 8);
    entry.DataPTR0 = (uint64_t)get_physaddr(m_IOSQ_address); // Set I/O Submission Queue address
    entry.CommandSpecific0 = ((0x003F0001) << 32) | (0x00010001); // 64 entries in queue, ID 1, IOCQ ID of 1, physically contiguous
    AdminCommand(&entry);

    fast_memset(&entry, 0, sizeof(NVMe::SubmissionQueueEntry) / 8);
    entry.command.Opcode = NVMe::IDENTIFY;
    void* m_ControllerInfo = WorldOS::g_KPM->AllocatePage();
    assert(m_ControllerInfo != nullptr);
    fast_memset(m_ControllerInfo, 0, 4096 / 8);
    entry.DataPTR0 = (uint64_t)get_physaddr(m_ControllerInfo); // Set I/O Submission Queue address
    entry.CommandSpecific0 = 1; // The controller
    AdminCommand(&entry);

    fast_memset(&entry, 0, sizeof(NVMe::SubmissionQueueEntry) / 8);
    entry.command.Opcode = NVMe::IDENTIFY;
    void* m_NSIDList = WorldOS::g_KPM->AllocatePage();
    assert(m_NSIDList != nullptr);
    fast_memset(m_NSIDList, 0, 4096 / 8);
    entry.DataPTR0 = (uint64_t)get_physaddr(m_NSIDList); // Set I/O Submission Queue address
    entry.CommandSpecific0 = 2; // The namespace list
    AdminCommand(&entry);

    fast_memset(&entry, 0, sizeof(NVMe::SubmissionQueueEntry) / 8);
    entry.command.Opcode = NVMe::IDENTIFY;
    void* m_NSID1Info = WorldOS::g_KPM->AllocatePage();
    assert(m_NSID1Info != nullptr);
    fast_memset(m_NSID1Info, 0, 4096 / 8);
    entry.DataPTR0 = (uint64_t)get_physaddr(m_NSID1Info); // Set I/O Submission Queue address
    entry.NSID = 1;
    entry.CommandSpecific0 = 0; // a namespace
    AdminCommand(&entry);

    fprintf(VFS_DEBUG, "NVMe ready!\n");
}

void NVMeDisk::Read(uint8_t* buffer, uint64_t lba, uint64_t count) {
    
}

void NVMeDisk::Write(const uint8_t* buffer, uint64_t lba, uint64_t count) {

}

size_t NVMeDisk::GetSectorSize() {

}

const char* NVMeDisk::getVendorName() {

}

const char* NVMeDisk::getDeviceName() {

}

const char* NVMeDisk::getDeviceClass() {

}

const char* NVMeDisk::getDeviceSubClass() {

}

const char* NVMeDisk::getDeviceProgramInterface() {

}

/* Protected Methods */

void NVMeDisk::AdminCommand(void* SQEntry) {
    NVMe::SubmissionQueueEntry* SQ = (NVMe::SubmissionQueueEntry*)SQEntry;
    uint32_t admin_tail = *(uint32_t*)((uint64_t)m_BAR0 + sizeof(NVMe::BAR0));
    uint32_t old_admin_tail = admin_tail;
    NVMe::BAR0* BAR0 = (NVMe::BAR0*)m_BAR0;
    fast_memcpy((void*)(BAR0->ASQ + admin_tail * sizeof(NVMe::SubmissionQueueEntry)), SQEntry, sizeof(NVMe::SubmissionQueueEntry));
    if (admin_tail >= 64)
        admin_tail = 0;
    else
        admin_tail++;
    *(uint32_t*)((uint64_t)m_BAR0 + sizeof(NVMe::BAR0)) = admin_tail;

    NVMe::CompletionQueueEntry* CQ = nullptr;
    do {
        CQ = (NVMe::CompletionQueueEntry*)(BAR0->ACQ + old_admin_tail * sizeof(NVMe::CompletionQueueEntry));
    } while (!CQ->Phase);
    fast_memset(CQ, 0, sizeof(NVMe::CompletionQueueEntry));
}

