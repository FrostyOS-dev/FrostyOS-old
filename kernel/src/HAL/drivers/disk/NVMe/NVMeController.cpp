#include "NVMeController.hpp"

#include <Memory/PagingUtil.hpp>
#include <Memory/PageManager.hpp>

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
    

    NVMeController::NVMeController() : m_BAR0(nullptr), m_IRQ(0), m_admin_queue(nullptr), m_ControllerInfo(nullptr), m_NSIDList(nullptr) {

    }

    NVMeController::~NVMeController() {

    }

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
        BAR0->CC.Enable = 0; // Disable the controller
        do {
            assert(BAR0->CSTS.CFS == 0);
        } while (BAR0->CSTS.Ready != 0); // Wait until the controller is stopped
        BAR0->AQA = 0x003F003F; // 64 entries for Admin submission Queue and Admin completion queue
        m_admin_queue = new NVMeAdminQueue;
        m_admin_queue->Create((void*)((uint64_t)m_BAR0 + 0x1000), 64, 0);
        void* ACQ = m_admin_queue->GetCompletionQueue();
        assert(ACQ != nullptr);
        void* ASQ = m_admin_queue->GetSubmissionQueue();
        assert(ASQ != nullptr);
        BAR0->ACQ = (uint64_t)ACQ;
        BAR0->ASQ = (uint64_t)ASQ;
        BAR0->CC.IOCQES = 4; // 2^4 = 16 (sizeof(CompletionQueueEntry))
        BAR0->CC.IOSQES = 6; // 2^6 = 64 (sizeof(SubmissionQueueEntry))
        BAR0->CC.CSS = 0b000;
        BAR0->CC.MPS = 0;
        BAR0->CC.Enable = 1; // Start the controller
        do {
            assert(BAR0->CSTS.CFS == 0);
        } while (BAR0->CSTS.Ready != 1); // Wait until the controller is running
        NVMeIOQueue* IOQueue = new NVMeIOQueue;
        IOQueue->Create((void*)((uint64_t)m_BAR0 + 0x1000), 64, 1);
        NVMe::SubmissionQueueEntry entry;

        fast_memset(&entry, 0, sizeof(SubmissionQueueEntry) / 8);
        entry.command.Opcode = (uint8_t)AdminCommands::CREATE_IO_COMPLETION_QUEUE;
        void* IOCQ = IOQueue->GetCompletionQueue();
        assert(IOCQ != nullptr);
        entry.DataPTR0 = (uint64_t)get_physaddr(IOCQ);
        entry.CommandSpecific0 = (UINT64_C(0x003F0001) << 32) | (0x00000001); // 64 entries in queue, ID 1, physically contiguous
        assert(m_admin_queue->SendCommand(&entry));

        fast_memset(&entry, 0, sizeof(SubmissionQueueEntry) / 8);
        entry.command.Opcode = (uint8_t)AdminCommands::CREATE_IO_SUBMISSION_QUEUE;
        void* IOSQ = IOQueue->GetCompletionQueue();
        assert(IOSQ != nullptr);
        entry.DataPTR0 = (uint64_t)get_physaddr(IOSQ);
        entry.CommandSpecific0 = (UINT64_C(0x003F0001) << 32) | (0x00010001); // 64 entries in queue, ID 1, IOCQ ID 1, physically contiguous
        assert(m_admin_queue->SendCommand(&entry));

        fast_memset(&entry, 0, sizeof(SubmissionQueueEntry) / 8);
        entry.command.Opcode = (uint8_t)AdminCommands::IDENTIFY;
        m_ControllerInfo = (IdentifyController*)WorldOS::g_KPM->AllocatePage();
        assert(m_ControllerInfo != nullptr);
        fast_memset(m_ControllerInfo, 0, 4096 / 8);
        entry.DataPTR0 = (uint64_t)get_physaddr(m_ControllerInfo);
        entry.CommandSpecific0 = 1; // The controller
        assert(m_admin_queue->SendCommand(&entry));

        fast_memset(&entry, 0, sizeof(SubmissionQueueEntry) / 8);
        entry.command.Opcode = (uint8_t)AdminCommands::IDENTIFY;
        m_NSIDList = (uint32_t*)WorldOS::g_KPM->AllocatePage();
        assert(m_NSIDList != nullptr);
        fast_memset(m_NSIDList, 0, 4096 / 8);
        entry.DataPTR0 = (uint64_t)get_physaddr(m_NSIDList);
        entry.CommandSpecific0 = 2; // NSID list
        assert(m_admin_queue->SendCommand(&entry));

        uint_fast16_t NSIndex = 0;
        while (m_NSIDList[NSIndex]) {
            NVMeDisk* disk = new NVMeDisk(m_NSIDList[NSIndex], this, IOQueue);
            m_Disks.insert(disk);
        }
    }

    const char* NVMeController::getVendorName() {

    }

    const char* NVMeController::getDeviceName() {

    }

    const char* NVMeController::getDeviceClass() {
        return "Mass Storage Controller";
    }

    const char* NVMeController::getDeviceSubClass() {
        return "Non-Volatile Memory Controller";
    }

    const char* NVMeController::getDeviceProgramInterface() {
        return "NVM Express";
    }
}