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

#ifndef _ELF_HPP
#define _ELF_HPP

#include <stdint.h>
#include <stddef.h>

#include <Memory/VirtualPageManager.hpp>
#include <Memory/PageManager.hpp>
#include <Memory/VirtualRegion.hpp>

#include <Scheduling/Process.hpp>

struct ELF_Header64 {
    char magic[4]; // should be 0x7F, then 'ELF' in ASCII
    uint8_t bit_arch; // 1 for 32-bit, 2 for 64-bit
    uint8_t endianess; // 1 for little endian, 2 for big endian
    uint8_t HeaderVersion;
    uint8_t OSABI; // Usually 0 for System V
    uint64_t padding;
    uint16_t file_type; // 1 for relocatable, 2 for executable, 3 for shared, 4 for core
    uint16_t InstructionSet; // 0 for No Specific, 2 for Sparc, 3 for x86, 8 for MIPS, 0x14 for PowerPC, 0x28 for ARM, 0x2A for SuperH, 0x32 for IA-64, 0x3E for x86-64, 0xB7 for AArch64, 0xF3 for RISC-V
    uint32_t ELFVersion;
    uint64_t ProgramEntryPosition;
    uint64_t ProgramHeaderTablePosition;
    uint64_t SectionHeaderTablePosition;
    uint32_t Flags; // Architecture dependent. no defined flags for x86/x86-64
    uint16_t HeaderSize;
    uint16_t ProgramHeaderEntrySize;
    uint16_t ProgramHeaderEntryCount;
    uint16_t SectionHeaderEntrySize;
    uint16_t SectionHeaderEntryCount;
    uint16_t IndexOfSectionHeaderTableWithNames;
} __attribute__((packed));

struct ELF_ProgramHeader64 {
    uint32_t SegmentType; // 0 for ignore, 1 for load, 2 for dynamic, 3 for interp, 4 for note
    uint32_t Flags; // bit 1 for executable, bit 2 for writable, bit 4 for readable
    uint64_t OffsetWithinFile;
    uint64_t VirtualAddress; // p_vaddr
    uint64_t Reserved;
    uint64_t SizeInFile; // p_filesz
    uint64_t SizeInMemory; // p_memsz
    uint64_t RequiredAlignment; // must be a power of 2
} __attribute__((packed));

struct ELF_entry_data {
    int64_t argc; // should be int, but gets promoted to int64_t for simplicity
    char** argv;
    int64_t envc; // should be int, but gets promoted to int64_t for simplicity
    char** envv;
} __attribute__((packed));

enum class ELFError {
    SUCCESS,
    ALLOCATION_FAILED,
    INVALID_ELF,
    INTERNAL_ERROR
};

class ELF_Executable {
public:
    ELF_Executable(void* addr, size_t size);
    ~ELF_Executable();

    bool Load(ELF_entry_data* entry_data);
    bool Execute();

    ELFError GetLastError() const;

private:
    void End_Handler();

    void SetLastError(ELFError error);

private:
    void* m_addr;
    ELF_Header64* m_header;
    size_t m_fileSize;
    VirtualPageManager* m_VPM;
    PageManager* m_PM;
    Scheduling::Process* m_process;
    Scheduling::ProcessEntry_t m_entry;
    VirtualRegion m_region;
    ELF_entry_data m_entry_data;
    ELF_entry_data* m_new_entry_data;
    size_t m_entry_data_size;
    ELFError m_error;
};

#endif /* _ELF_HPP */