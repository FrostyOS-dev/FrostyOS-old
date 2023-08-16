/*
Copyright (©) 2023  Frosty515

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

#ifndef _HAL_NVME_CONTROLLER_HPP
#define _HAL_NVME_CONTROLLER_HPP

#include "../../PCIDevice.hpp"

#include "NVMeAdminQueue.hpp"
#include "NVMeIOQueue.hpp"
#include "NVMeDisk.hpp"

#include <Data-structures/LinkedList.hpp>

namespace NVMe {
    struct IdentifyController; // Defined in C++ source file.

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
        uint32_t Reserved : 25;
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

    class NVMeController : public PCIDevice {
    public:
        NVMeController();
        ~NVMeController() override;

        void InitPCIDevice(PCI::Header0* device) override;

        IdentifyNamespace* IdentifyDisk(uint32_t ID) const;

        const char* getVendorName() const override;
        const char* getDeviceName() const override;

        const char* getDeviceClass() const override;
        const char* getDeviceSubClass() const override;
        const char* getDeviceProgramInterface() const override;
    private:
        void* m_BAR0;
        uint8_t m_IRQ;
        NVMeAdminQueue* m_admin_queue;
        IdentifyController* m_ControllerInfo;
        uint32_t* m_NSIDList;
        LinkedList::SimpleLinkedList<NVMeIOQueue> m_IOQueues;
        LinkedList::SimpleLinkedList<NVMeDisk> m_Disks;
    };

}

#endif /* _HAL_NVME_CONTROLLER_HPP */