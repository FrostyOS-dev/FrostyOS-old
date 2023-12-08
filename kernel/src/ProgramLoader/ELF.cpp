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

#include "ELF.hpp"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <util.h>
#include <math.h>

#include <Scheduling/Thread.hpp>

#ifdef __x86_64__
#include <arch/x86_64/io.h>
#include <arch/x86_64/Memory/PagingUtil.hpp>
#endif

ELF_Executable::ELF_Executable(void* addr, size_t size) : m_addr(addr), m_header(nullptr), m_fileSize(size), m_VPM(nullptr), m_process(nullptr), m_entry(nullptr), m_entry_data({0, nullptr, 0, nullptr}), m_new_entry_data(nullptr), m_entry_data_size(0), m_error(ELFError::SUCCESS) {

}

ELF_Executable::~ELF_Executable() {
    if (m_new_entry_data != nullptr && m_PM != nullptr) {
        if (m_entry_data_size > PAGE_SIZE)
            m_PM->FreePages(m_new_entry_data);
        else
            m_PM->FreePage(m_new_entry_data);
    }
    if (m_VPM != nullptr) {
        delete m_VPM;
        delete m_PM;
    }
    if (m_process != nullptr)
        delete m_process;
}

bool ELF_Executable::Load(ELF_entry_data* entry_data) {
    {
        ELF_Header64* header = (ELF_Header64*)m_addr;
        char magic[4] = { 0x7F, 'E', 'L', 'F' };
        if (memcmp(magic, header->magic, 4) != 0) {
            SetLastError(ELFError::INVALID_ELF);
            return false; // invalid magic
        }
        if (header->file_type != 2 || header->bit_arch != 2 || header->endianess != 1 || header->InstructionSet != 0x3e) { // must be an Executable, 64-bit, little-endian and x86_64
            SetLastError(ELFError::INVALID_ELF);
            return false;
        }
        m_header = header;
    }
    void* lowest_addr = (void*)UINT64_MAX;
    void* highest_addr = nullptr;
    ELF_ProgramHeader64** prog_headers = (ELF_ProgramHeader64**)((uint64_t)m_header + m_header->ProgramHeaderTablePosition);
    for (uint64_t i = 0; i < m_header->ProgramHeaderEntryCount; i++) {
        ELF_ProgramHeader64* prog_header = (ELF_ProgramHeader64*)((uint64_t)prog_headers + i * m_header->ProgramHeaderEntrySize);
        switch (prog_header->SegmentType) {
            case 0: // ignore
            case 4: // note
                continue;
            case 1: {
                // FIXME: change alignment to use shifting instead of multiplication and division as the alignment must be a power of 2
                uint8_t Alignment = log2(prog_header->RequiredAlignment);
                void* virt_addr = (void*)(((uint64_t)(prog_header->VirtualAddress) >> Alignment) << Alignment);
                //void* virt_addr = (void*)(ALIGN_DOWN(prog_header->VirtualAddress, prog_header->RequiredAlignment));
                if (virt_addr < lowest_addr)
                    lowest_addr = virt_addr;
                void* end_addr = (void*)((((uint64_t)virt_addr + prog_header->SizeInMemory + (prog_header->RequiredAlignment - 1)) >> Alignment) << Alignment);
                //void* end_addr = (void*)(ALIGN_UP(((uint64_t)virt_addr + prog_header->SizeInMemory), prog_header->RequiredAlignment));
                if (end_addr > highest_addr)
                    highest_addr = end_addr;
                break;
            }
            default: // unknown
                SetLastError(ELFError::INVALID_ELF);
                return false;
        }
    }
    m_region = VirtualRegion((void*)0x1000, (void*)0x800000000000);
    m_VPM = new VirtualPageManager;
    if (m_VPM == nullptr)
        return false;
    m_VPM->InitVPageMgr(m_region);
    m_PM = new PageManager(m_region, m_VPM, true, true);
#ifdef __x86_64__
    uint64_t old_CR3 = x86_64_SwapCR3((uint64_t)(m_PM->GetPageTable().GetRootTablePhysical()) & 0x000FFFFFFFFFF000);
#endif
    for (uint64_t i = 0; i < m_header->ProgramHeaderEntryCount; i++) {
        ELF_ProgramHeader64* prog_header = (ELF_ProgramHeader64*)((uint64_t)prog_headers + i * m_header->ProgramHeaderEntrySize);
        switch (prog_header->SegmentType) {
            case 0: // ignore
            case 4: // note
                continue;
            case 1: {
                PagePermissions perms;
                if (prog_header->Flags & 1) { // NOTE: WRITE_EXECUTE and READ_WRITE_EXECUTE are unsupported
                    if (prog_header->Flags & 4)
                        perms = PagePermissions::READ_EXECUTE;
                    else
                        perms = PagePermissions::EXECUTE;
                }
                else if (prog_header->Flags & 2) {
                    if (prog_header->Flags & 4)
                        perms = PagePermissions::READ_WRITE;
                    else
                        perms = PagePermissions::WRITE;
                }
                else
                    perms = PagePermissions::READ;
                uint8_t Alignment = log2(prog_header->RequiredAlignment);
                uint64_t RequiredAlignment = ALIGN_UP(prog_header->RequiredAlignment, 8);
                void* start = ALIGN_ADDRESS_DOWN(prog_header->VirtualAddress, 8);
                //uint64_t mem_size = ALIGN_UP(prog_header->SizeInMemory, RequiredAlignment);
                uint64_t mem_size = ((prog_header->SizeInMemory + (RequiredAlignment - 1)) >> Alignment) << Alignment;
                uint64_t file_size = prog_header->SizeInFile;
                uint64_t page_count = DIV_ROUNDUP(mem_size, PAGE_SIZE);
                uint8_t PageAlignment = Alignment < 12 ? 12 : Alignment;
                void* page_start = (void*)(((uint64_t)start >> PageAlignment) << PageAlignment);
                //void* page_start = ALIGN_ADDRESS_DOWN(start, (ALIGN_UP(RequiredAlignment, PAGE_SIZE)));
#ifdef __x86_64__
                x86_64_DisableInterrupts();
#endif
                void* data = m_PM->AllocatePages(page_count, PagePermissions::READ_WRITE, page_start);
                if (data == nullptr) {
#ifdef __x86_64__
                    x86_64_EnableInterrupts();
#endif
                    SetLastError(ELFError::ALLOCATION_FAILED);
                    return false;
                }
                fast_memset(page_start, 0, mem_size >> 3);
                fast_memcpy(start, (void*)(ALIGN_DOWN(((uint64_t)m_addr + prog_header->OffsetWithinFile), 8)), ALIGN_UP(file_size, 8));
                m_PM->Remap(page_start, perms);
#ifdef __x86_64__
                x86_64_EnableInterrupts();
#endif
                break;
            }
            default: // unknown
                SetLastError(ELFError::INVALID_ELF);
                return false;
        }
    }
    m_entry = (Scheduling::ProcessEntry_t)(m_header->ProgramEntryPosition);
    fast_memcpy(&m_entry_data, entry_data, sizeof(ELF_entry_data));
    uint64_t total_size = sizeof(ELF_entry_data);
    total_size += (m_entry_data.argc + 1) * sizeof(char*);
    total_size += (m_entry_data.envc + 1) * sizeof(char*);
    for (int64_t i = 0; i < m_entry_data.argc; i++)
        total_size += strlen(m_entry_data.argv[i]) + 1;
    total_size += 1;
    for (int64_t i = 0; i < m_entry_data.envc; i++)
        total_size += strlen(m_entry_data.envv[i]) + 1;
    total_size += 1;
    char* entry_data_address = (char*)m_PM->AllocatePages(DIV_ROUNDUP(total_size, PAGE_SIZE));
    if (entry_data_address == nullptr) {
        SetLastError(ELFError::ALLOCATION_FAILED);
        return false;
    }
    ELF_entry_data* new_entry_data = (ELF_entry_data*)entry_data_address;
    fast_memset(entry_data_address, 0, (ALIGN_UP(total_size, PAGE_SIZE)) >> 3);
    uint64_t current_offset = 0;
    fast_memcpy(entry_data_address, &m_entry_data, sizeof(ELF_entry_data));
    current_offset += sizeof(ELF_entry_data);
    new_entry_data->argv = (char**)&(entry_data_address[current_offset]);
    fast_memcpy(&(entry_data_address[current_offset]), m_entry_data.argv, m_entry_data.argc * sizeof(char*));
    current_offset += (m_entry_data.argc + 1) * sizeof(char*);
    new_entry_data->envv = (char**)&(entry_data_address[current_offset]);
    fast_memcpy(&(entry_data_address[current_offset]), m_entry_data.envv, m_entry_data.envc * sizeof(char*));
    current_offset += (m_entry_data.envc + 1) * sizeof(char*);
    for (int64_t i = 0; i < m_entry_data.argc; i++, current_offset++) {
        uint64_t j = 0;
        char c = m_entry_data.argv[i][j];
        new_entry_data->argv[i] = &(entry_data_address[current_offset]);
        while (1) {
            entry_data_address[current_offset] = c;
            j++;
            current_offset++;
            if (c == 0) // still copy the termination and increment counters
                break;
            c = m_entry_data.argv[i][j];
        }
    }
    entry_data_address[current_offset] = 0;
    current_offset++;
    for (int64_t i = 0; i < m_entry_data.envc; i++, current_offset++) {
        uint64_t j = 0;
        char c = m_entry_data.envv[i][j];
        new_entry_data->envv[i] = &(entry_data_address[current_offset]);
        while (1) {
            entry_data_address[current_offset] = c;
            j++;
            current_offset++;
            if (c == 0) // still copy the termination and increment counters
                break;
            c = m_entry_data.envv[i][j];
        }
    }
    entry_data_address[current_offset] = 0;
    current_offset++;
    fast_memcpy(&m_entry_data, new_entry_data, sizeof(ELF_entry_data));
    m_new_entry_data = new_entry_data;
    m_entry_data_size = total_size;
#ifdef __x86_64__
    x86_64_LoadCR3(old_CR3);
#endif
    SetLastError(ELFError::SUCCESS);
    return true;
}

bool ELF_Executable::Execute(Scheduling::Priority priority) {
    if (m_VPM == nullptr || m_PM == nullptr) {
        SetLastError(ELFError::INTERNAL_ERROR);
        return false;
    }
    m_process = new Scheduling::Process;
    m_process->SetPageManager(m_PM);
    m_process->SetVirtualPageManager(m_VPM);
    m_process->SetRegion(m_region);
    m_process->SetFlags(Scheduling::USER_DEFAULT);
    m_process->SetEntry(m_entry, m_new_entry_data);
    m_process->SetPriority(priority);
    m_process->SetUID(0);
    m_process->SetGID(0);
    m_process->CreateMainThread();
    m_process->GetMainThread()->SetCleanupFunction({(void (*)(void*))&ELF_Executable::End_Handler, (void*)this});
    m_process->Start();
    SetLastError(ELFError::SUCCESS);
    return true;
}

ELFError ELF_Executable::GetLastError() const {
    return m_error;
}

void ELF_Executable::End_Handler() {
    m_process = nullptr;
    if (m_new_entry_data != nullptr && m_PM != nullptr) {
        if (m_entry_data_size > PAGE_SIZE)
            m_PM->FreePages(m_new_entry_data);
        else
            m_PM->FreePage(m_new_entry_data);
    }
    if (m_VPM != nullptr) {
        delete m_PM;
        delete m_VPM;
    }
    kfree(this);
}

void ELF_Executable::SetLastError(ELFError error) {
    m_error = error;
}
