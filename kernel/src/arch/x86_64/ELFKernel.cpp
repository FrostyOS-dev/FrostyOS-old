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

#include "ELFKernel.hpp"

#include "Memory/PageMapIndexer.hpp"

bool MapKernel(void* kernel_phys, void* kernel_virt, size_t kernel_size) {
    // page aligned start addresses
    uint64_t text_start_addr = (uint64_t)_text_start_addr;
    text_start_addr -= text_start_addr % 4096;
    uint64_t rodata_start_addr = (uint64_t)_rodata_start_addr;
    rodata_start_addr -= rodata_start_addr % 4096;
    uint64_t data_start_addr = (uint64_t)_data_start_addr;
    data_start_addr -= data_start_addr % 4096;
    uint64_t bss_start_addr = (uint64_t)_bss_start_addr;
    bss_start_addr -= bss_start_addr % 4096;

    uint64_t text_size = (uint64_t)_text_end_addr - text_start_addr;
    uint64_t rodata_size = (uint64_t)_rodata_end_addr - rodata_start_addr;
    uint64_t data_size = (uint64_t)_data_end_addr - data_start_addr;
    uint64_t bss_size = (uint64_t)_bss_end_addr - bss_start_addr;

    // this just makes sure nothing is cut off
    text_size   += 4095;
    rodata_size += 4095;
    data_size   += 4095;
    bss_size    += 4095;

    // convert to pages
    text_size   /= 4096;
    rodata_size /= 4096;
    data_size   /= 4096;
    bss_size    /= 4096;

    // pre-map kernel as Read-only and No execute in case there are more unused sections
    kernel_size += 4095;
    kernel_size /= 4096;

    for (uint64_t i = 0; i < kernel_size; i++)
        x86_64_map_page_noflush(&K_PML4_Array, (void*)((uint64_t)kernel_phys + (i * 4096)), (void*)((uint64_t)kernel_virt + (i * 4096)), 0x8000001);

    // actually do the mapping
    for (uint64_t i = 0; i < text_size; i++)
        x86_64_map_page_noflush(&K_PML4_Array, (void*)(text_start_addr + (i * 4096) - (uint64_t)kernel_virt + (uint64_t)kernel_phys), (void*)(text_start_addr + (i * 4096)), 0x1); // Present, Read-only, Execute

    for (uint64_t i = 0; i < rodata_size; i++)
        x86_64_map_page_noflush(&K_PML4_Array, (void*)(rodata_start_addr + (i * 4096) - (uint64_t)kernel_virt + (uint64_t)kernel_phys), (void*)(rodata_start_addr + (i * 4096)), 0x8000001); // Present, Read-only, Execute Disable

    for (uint64_t i = 0; i < data_size; i++)
        x86_64_map_page_noflush(&K_PML4_Array, (void*)(data_start_addr + (i * 4096) - (uint64_t)kernel_virt + (uint64_t)kernel_phys), (void*)(data_start_addr + (i * 4096)), 0x8000003); // Present, Read-Write, Execute Disable

    for (uint64_t i = 0; i < bss_size; i++)
        x86_64_map_page_noflush(&K_PML4_Array, (void*)(bss_start_addr + (i * 4096) - (uint64_t)kernel_virt + (uint64_t)kernel_phys), (void*)(bss_start_addr + (i * 4096)), 0x8000003); // Present, Read-Write, Execute Disable

    return true;
}