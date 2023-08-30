#include "ELF.hpp"

#include <string.h>
#include <stdio.h>
#include <util.h>



#ifdef __x86_64__
#include <arch/x86_64/io.h>
#endif

ELF_Executable::ELF_Executable(void* addr, size_t size) : m_addr(addr), m_header(nullptr), m_fileSize(size), m_VPM(nullptr), m_process(nullptr), m_entry(nullptr) {

}

ELF_Executable::~ELF_Executable() {
    if (m_VPM != nullptr) {
        delete m_VPM;
        delete m_PM;
    }
    if (m_process != nullptr)
        delete m_process;
}

bool ELF_Executable::Load() {
    {
        ELF_Header64* header = (ELF_Header64*)m_addr;
        char magic[4] = { 0x7F, 'E', 'L', 'F' };
        if (memcmp(magic, header->magic, 4) != 0)
            return false; // invalid magic
        if (header->file_type != 2 || header->bit_arch != 2 || header->endianess != 1 || header->InstructionSet != 0x3e) // must be an Executable, 64-bit, little-endian and x86_64
            return false;
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
                void* virt_addr = (void*)(ALIGN_DOWN(prog_header->VirtualAddress, prog_header->RequiredAlignment));
                if (virt_addr < lowest_addr)
                    lowest_addr = virt_addr;
                void* end_addr = (void*)(ALIGN_UP(((uint64_t)virt_addr + prog_header->SizeInMemory), prog_header->RequiredAlignment));
                if (end_addr > highest_addr)
                    highest_addr = end_addr;
                break;
            }
            default: // unknown
                return false;
        }
    }
    size_t page_count = 0;
    if (highest_addr == lowest_addr)
        page_count = 1;
    else
        page_count = DIV_ROUNDUP(((uint64_t)highest_addr - (uint64_t)lowest_addr), PAGE_SIZE);
    void* pages = WorldOS::g_VPM->AllocatePages(lowest_addr, page_count);
    if (pages != lowest_addr)
        return false;
    m_region = WorldOS::VirtualRegion(lowest_addr, highest_addr);
    m_VPM = new WorldOS::VirtualPageManager;
    if (m_VPM == nullptr)
        return false;
    m_VPM->InitVPageMgr(m_region);
    m_PM = new WorldOS::PageManager(m_region, m_VPM, true, true);
    for (uint64_t i = 0; i < m_header->ProgramHeaderEntryCount; i++) {
        ELF_ProgramHeader64* prog_header = (ELF_ProgramHeader64*)((uint64_t)prog_headers + i * m_header->ProgramHeaderEntrySize);
        switch (prog_header->SegmentType) {
            case 0: // ignore
            case 4: // note
                continue;
            case 1: {
                using namespace WorldOS;
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
                uint64_t RequiredAlignment = ALIGN_UP(prog_header->RequiredAlignment, 8);
                void* start = (void*)(ALIGN_DOWN(((uint64_t)prog_header->VirtualAddress), RequiredAlignment));
                uint64_t mem_size = ALIGN_UP(prog_header->SizeInMemory, RequiredAlignment);
                uint64_t file_size = prog_header->SizeInFile;
                uint64_t page_count = DIV_ROUNDUP(mem_size, PAGE_SIZE);
                void* page_start = (void*)(ALIGN_DOWN(((uint64_t)start), PAGE_SIZE));
#ifdef __x86_64__
                x86_64_DisableInterrupts();
#endif
                void* data = m_PM->AllocatePages(page_count, PagePermissions::READ_WRITE, page_start);
                if (data == nullptr) {
#ifdef __x86_64__
                    x86_64_EnableInterrupts();
#endif
                    return false;
                }
                fast_memset(start, 0, mem_size >> 3);
                fast_memcpy(start, (void*)(ALIGN_DOWN(((uint64_t)m_addr + prog_header->OffsetWithinFile), 8)), ALIGN_UP(file_size, 8));
                if (page_count > 1)
                    m_PM->FreePages(data);
                else
                    m_PM->FreePage(data);
                data = m_PM->AllocatePages(page_count, perms, page_start);
                if (data == nullptr) { // this is very bad. We have copied the data, but we could not allocate the region again. This should NEVER happen, and if it does, something is very wrong
#ifdef __x86_64__
                    x86_64_EnableInterrupts();
#endif
                    fprintf(VFS_DEBUG, "[%s] WARNING: second allocation failed. Something very serious has gone wrong. This should be impossible.\n", __extension__ __PRETTY_FUNCTION__);
                    return false;
                }
#ifdef __x86_64__
                x86_64_EnableInterrupts();
#endif
                break;
            }
            default: // unknown
                return false;
        }
    }
    m_entry = (Scheduling::ProcessEntry_t)(m_header->ProgramEntryPosition);
    return true;
}

bool ELF_Executable::Execute() {
    if (m_VPM == nullptr || m_PM == nullptr)
        return false;
    m_process = new Scheduling::Process;
    m_process->SetPageManager(m_PM);
    m_process->SetVirtualPageManager(m_VPM);
    m_process->SetRegion(m_region);
    m_process->SetFlags(Scheduling::USER_DEFAULT);
    m_process->SetEntry(m_entry, nullptr);
    m_process->SetPriority(Scheduling::Priority::NORMAL);
    m_process->Start();
    return true;
}
