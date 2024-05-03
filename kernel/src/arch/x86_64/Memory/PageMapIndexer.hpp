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

#ifndef _KERNEL_X86_64_PAGE_MAP_INDEXER_HPP
#define _KERNEL_X86_64_PAGE_MAP_INDEXER_HPP

#include "PageTables.hpp"

#include <stdint.h>
#include <stddef.h>

// Get the page-aligned physical address from a page-aligned virtual address.
void* x86_64_get_physaddr(Level4Group* PML4Array, void* virtualaddr);

// Get the HHDM version of an address under 512GiB
void* x86_64_to_HHDM(void* physaddr);

// Map a page. physaddr and virtualaddr must be page aligned. Can potentially map 512 pages instead of just 1 due to x86_64 limitations.
void x86_64_map_page(Level4Group* PML4Array, void* physaddr, void* virtualaddr, uint32_t flags);

// Map a page with no TLB flush. physaddr and virtualaddr must be page aligned.
void x86_64_map_page_noflush(Level4Group* PML4Array, void* physaddr, void* virtualaddr, uint32_t flags);

// Unmap a page. virtualaddr must be page aligned.
void x86_64_unmap_page(Level4Group* PML4Array, void* virtualaddr);

// Unmap a page with no TLB flush
void x86_64_unmap_page_noflush(Level4Group* PML4Array, void* virtualaddr);

// Update flags of page mapping
void x86_64_remap_page(Level4Group* PML4Array, void* virtualaddr, uint32_t flags);

// Update flags of page mapping with no TLB flush
void x86_64_remap_page_noflush(Level4Group* PML4Array, void* virtualaddr, uint32_t flags);

// Identity map memory. If length and/or start_phys aren't page aligned, the values used are rounded down to the nearest standard page boundary.
void x86_64_identity_map(Level4Group* PML4Array, void* start_phys, uint64_t length, uint32_t flags);

// Map a 2MiB page with no TLB flush. Both address will be rounded down to nearest 2MiB border
void x86_64_map_large_page_noflush(Level4Group* PML4Array, void* physaddr, void* virtaddr, uint32_t flags);

// Map a 2MiB page. Both address will be rounded down to nearest 2MiB border
void x86_64_map_large_page(Level4Group* PML4Array, void* physaddr, void* virtaddr, uint32_t flags);

// Unmap a 2MiB page with no TLB flush. virtualaddr will be rounded down to nearest 2MiB border
void x86_64_unmap_large_page_noflush(Level4Group* PML4Array, void* virtualaddr);

// Unmap a 2MiB page. virtualaddr will be rounded down to nearest 2MiB border
void x86_64_unmap_large_page(Level4Group* PML4Array, void* virtualaddr);

// Update flags of large page mapping
void x86_64_remap_large_page(Level4Group* PML4Array, void* virtualaddr, uint32_t flags);

// Update flags of large page mapping with no TLB flush
void x86_64_remap_large_page_noflush(Level4Group* PML4Array, void* virtualaddr, uint32_t flags);

// Set the virtual and physical addresses of the kernel
void x86_64_SetKernelAddress(void* kernel_virtual, void* kernel_physical, size_t length);

// Set the HHDM start address
void x86_64_SetHHDMStart(void* virtualaddr);

// Get the HHDM start address
void* x86_64_GetHHDMStart();

extern Level4Group K_PML4_Array;

#endif /* _KERNEL_X86_64_PAGE_MAP_INDEXER_HPP */