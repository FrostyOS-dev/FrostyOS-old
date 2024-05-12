/*
Copyright (©) 2022-2023  Frosty515

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

#ifndef _KERNEL_HPP
#define _KERNEL_HPP

#include <stdint.h>
#include <stddef.h>

#include <Memory/Memory.hpp>
#include <Graphics/Graphics.h>

constexpr uint64_t EARLY_STAGE = 0x5354414745000000;
constexpr uint64_t STAGE2      = 0x5354414745000001;
constexpr uint64_t USER_STAGE  = 0x5354414745000002;

struct KernelParams {
    FrameBuffer frameBuffer;
    MemoryMapEntry** MemoryMap;
    size_t MemoryMapEntryCount;
    void* EFI_SYSTEM_TABLE_ADDR;
    uint64_t kernel_physical_addr;
    uint64_t kernel_virtual_addr;
    size_t kernel_size;
    void* RSDP_table;
    uint64_t hhdm_start_addr;
    void* initramfs_addr;
    size_t initramfs_size;
};

extern "C" void StartKernel(KernelParams* params);
void Kernel_Stage2(void*);

namespace Scheduling {
    class Process;
}

extern Scheduling::Process* KProcess;

#endif /* _KERNEL_HPP */
