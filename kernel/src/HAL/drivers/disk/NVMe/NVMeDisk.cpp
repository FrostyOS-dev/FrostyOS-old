#include "NVMeDisk.hpp"
#include "NVMeController.hpp"

#include <Memory/PagingUtil.hpp>
#include <Memory/PageManager.hpp>

#include <assert.h>
#include <stdio.hpp>
#include <util.h>

namespace NVMe {

    NVMeDisk::NVMeDisk(uint32_t ID, NVMeController* controller, NVMeIOQueue* IOQueue, size_t MaxTransferSize) : m_ID(ID), m_controller(controller), m_IOQueue(IOQueue), m_MaxTransferSize(MaxTransferSize), m_SectorSize(0), m_info(nullptr) {
        m_info = m_controller->IdentifyDisk(m_ID);
        assert(m_info != nullptr);
        assert(m_info->LBAF[0].LBADS >= 9);
        m_SectorSize = UINT64_C(1) << m_info->LBAF[0].LBADS;
    }

    NVMeDisk::~NVMeDisk() {

    }

    bool NVMeDisk::Read(uint8_t* buffer, uint64_t lba, uint64_t count) {
        if (buffer == nullptr)
            return false;
        if (count == 0)
            return false;
        uint8_t* out_buffer = nullptr;
        SubmissionQueueEntry entry;
        fast_memset(&entry, 0, sizeof(SubmissionQueueEntry) / 8);
        entry.NSID = m_ID;
        entry.command.Opcode = (uint8_t)IOCommands::READ;
        entry.CommandSpecific0 = lba;
        entry.CommandSpecific1 = (uint16_t)(count - 1); // max count is 65536
        LinkedList::SimpleLinkedList<uint64_t> PRPLists;
        uint64_t page_count = DIV_ROUNDUP(count * m_SectorSize, 0x1000);
        if (page_count == 1) {
            out_buffer = (uint8_t*)WorldOS::g_KPM->AllocatePage();
            if (out_buffer == nullptr)
                return false;
            entry.DataPTR0 = (uint64_t)get_physaddr(out_buffer);
        }
        else {
            out_buffer = (uint8_t*)WorldOS::g_KPM->AllocatePages(page_count);
            if (out_buffer == nullptr)
                return false;
            entry.DataPTR0 = (uint64_t)get_physaddr(out_buffer);
            if (page_count == 2) {
                entry.DataPTR1 = (uint64_t)get_physaddr((void*)((uint64_t)out_buffer + 0x1000));
            }
            else {
                uint64_t PRPListCount = DIV_ROUNDUP((page_count - 2), 511); // first page is in entry.DataPTR0, the last PRP list has 512 entries
                uint64_t* PRPList = nullptr;
                uint64_t* new_PRPList = nullptr;
                for (uint64_t i = 0; i < (PRPListCount - 1); i++) {
                    new_PRPList = (uint64_t*)WorldOS::g_KPM->AllocatePage();
                    if (new_PRPList == nullptr) {
                        for (uint64_t i = 0; i < PRPLists.getCount(); i++) {
                            WorldOS::g_KPM->FreePage(PRPLists.get(i));
                            PRPLists.remove(i);
                        }
                        WorldOS::g_KPM->FreePages(out_buffer);
                        return false;
                    }
                    PRPLists.insert(new_PRPList);
                    if (PRPList != nullptr)
                        PRPList[511] = (uint64_t)get_physaddr(new_PRPList);
                    PRPList = new_PRPList;
                    for (uint64_t j = 0; j < 510; j++)
                        PRPList[j] = (uint64_t)get_physaddr((void*)((uint64_t)out_buffer + 0x1000 + (i * 511 + j) * 0x1000));
                }
                new_PRPList = (uint64_t*)WorldOS::g_KPM->AllocatePage();
                if (new_PRPList == nullptr) {
                    for (uint64_t i = 0; i < PRPLists.getCount(); i++) {
                        WorldOS::g_KPM->FreePage(PRPLists.get(i));
                        PRPLists.remove(i);
                    }
                    WorldOS::g_KPM->FreePages(out_buffer);
                    return false;
                }
                PRPLists.insert(new_PRPList);
                if (PRPList != nullptr)
                    PRPList[511] = (uint64_t)get_physaddr(new_PRPList);
                PRPList = new_PRPList;
                for (uint64_t i = 0; i < 511; i++)
                    PRPList[i] = (uint64_t)get_physaddr((void*)((uint64_t)out_buffer + 0x1000 + ((PRPListCount - 1) * 511 + i) * 0x1000));
            }
        }
        bool successful = m_IOQueue->SendCommand(&entry);
        if (successful)
            fast_memcpy(buffer, out_buffer, page_count * 4096 / 8);
        if (page_count == 1) {
            WorldOS::g_KPM->FreePage(out_buffer);
        }
        else {
            WorldOS::g_KPM->FreePages(out_buffer);
            if (page_count != 2) {
                for (uint64_t i = 0; i < PRPLists.getCount(); i++) {
                    WorldOS::g_KPM->FreePage(PRPLists.get(i));
                    PRPLists.remove(i);
                }
            }
        }
        return successful;
    }

    bool NVMeDisk::Write(const uint8_t* buffer, uint64_t lba, uint64_t count) {
        if (buffer == nullptr)
            return false;
        if (count == 0)
            return false;
        uint8_t* out_buffer = nullptr;
        SubmissionQueueEntry entry;
        fast_memset(&entry, 0, sizeof(SubmissionQueueEntry) / 8);
        entry.NSID = m_ID;
        entry.command.Opcode = (uint8_t)IOCommands::WRITE;
        entry.CommandSpecific0 = lba;
        entry.CommandSpecific1 = (uint16_t)(count - 1); // max count is 65536
        LinkedList::SimpleLinkedList<uint64_t> PRPLists;
        uint64_t page_count = DIV_ROUNDUP(count * m_SectorSize, 0x1000);
        if (page_count == 1) {
            out_buffer = (uint8_t*)WorldOS::g_KPM->AllocatePage();
            if (out_buffer == nullptr)
                return false;
            fast_memcpy(out_buffer, buffer, 4096 / 8);
            entry.DataPTR0 = (uint64_t)get_physaddr(out_buffer);
        }
        else {
            out_buffer = (uint8_t*)WorldOS::g_KPM->AllocatePages(page_count);
            if (out_buffer == nullptr)
                return false;
            fast_memcpy(out_buffer, buffer, page_count * 4096 / 8);
            entry.DataPTR0 = (uint64_t)get_physaddr(out_buffer);
            if (page_count == 2) {
                entry.DataPTR1 = (uint64_t)get_physaddr((void*)((uint64_t)out_buffer + 0x1000));
            }
            else {
                uint64_t PRPListCount = DIV_ROUNDUP((page_count - 2), 511); // first page is in entry.DataPTR0, the last PRP list has 512 entries
                uint64_t* PRPList = nullptr;
                uint64_t* new_PRPList = nullptr;
                for (uint64_t i = 0; i < (PRPListCount - 1); i++) {
                    new_PRPList = (uint64_t*)WorldOS::g_KPM->AllocatePage();
                    if (new_PRPList == nullptr) {
                        for (uint64_t i = 0; i < PRPLists.getCount(); i++) {
                            WorldOS::g_KPM->FreePage(PRPLists.get(i));
                            PRPLists.remove(i);
                        }
                        WorldOS::g_KPM->FreePages(out_buffer);
                        return false;
                    }
                    PRPLists.insert(new_PRPList);
                    if (PRPList != nullptr)
                        PRPList[511] = (uint64_t)get_physaddr(new_PRPList);
                    PRPList = new_PRPList;
                    for (uint64_t j = 0; j < 510; j++)
                        PRPList[j] = (uint64_t)get_physaddr((void*)((uint64_t)out_buffer + 0x1000 + (i * 511 + j) * 0x1000));
                }
                new_PRPList = (uint64_t*)WorldOS::g_KPM->AllocatePage();
                if (new_PRPList == nullptr) {
                    for (uint64_t i = 0; i < PRPLists.getCount(); i++) {
                        WorldOS::g_KPM->FreePage(PRPLists.get(i));
                        PRPLists.remove(i);
                    }
                    WorldOS::g_KPM->FreePages(out_buffer);
                    return false;
                }
                PRPLists.insert(new_PRPList);
                if (PRPList != nullptr)
                    PRPList[511] = (uint64_t)get_physaddr(new_PRPList);
                PRPList = new_PRPList;
                for (uint64_t i = 0; i < 511; i++)
                    PRPList[i] = (uint64_t)get_physaddr((void*)((uint64_t)out_buffer + 0x1000 + ((PRPListCount - 1) * 511 + i) * 0x1000));
            }
        }
        bool successful = m_IOQueue->SendCommand(&entry);
        if (page_count == 1) {
            WorldOS::g_KPM->FreePage(out_buffer);
        }
        else {
            WorldOS::g_KPM->FreePages(out_buffer);
            if (page_count != 2) {
                for (uint64_t i = 0; i < PRPLists.getCount(); i++) {
                    WorldOS::g_KPM->FreePage(PRPLists.get(i));
                    PRPLists.remove(i);
                }
            }
        }
        return successful;
    }

    size_t NVMeDisk::GetSectorSize() {
        return m_SectorSize;
    }

}
