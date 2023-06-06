#ifndef _KERNEL_X86_64_PAGE_MAP_INDEXER_HPP
#define _KERNEL_X86_64_PAGE_MAP_INDEXER_HPP

#include "PageTables.hpp"
#include "PagingUtil.hpp"

#include <stdint.h>
#include <stddef.h>

// Get the page-aligned physical address from a page-aligned virtual address.
void* x86_64_get_physaddr(void* virtualaddr);

// Get the HHDM version of an address under 512GiB
void* x86_64_to_HHDM(void* physaddr);

// Map a page. physaddr and virtualaddr must be page aligned. Can potentially map 512 pages instead of just 1 due to x86_64 limitations.
void x86_64_map_page(void* physaddr, void* virtualaddr, uint32_t flags);

// Map a page with no TLB flush. physaddr and virtualaddr must be page aligned.
void x86_64_map_page_noflush(void* physaddr, void* virtualaddr, uint32_t flags);

// Unmap a page. virtualaddr must be page aligned.
void x86_64_unmap_page(void* virtualaddr);

// Unmap a page with no TLB flush
void x86_64_unmap_page_noflush(void* virtualaddr);

// Identity map memory. If length and/or start_phys aren't page aligned, the values used are rounded down to the nearest standard page boundary.
void x86_64_identity_map(void* start_phys, uint64_t length, uint32_t flags);

// Map a 2MiB page with no TLB flush. Both address will be rounded down to nearest 2MiB border
void x86_64_map_large_page_noflush(void* physaddr, void* virtaddr, uint32_t flags);

// Map a 2MiB page. Both address will be rounded down to nearest 2MiB border
void x86_64_map_large_page(void* physaddr, void* virtaddr, uint32_t flags);

// Unmap a 2MiB page with no TLB flush. virtualaddr will be rounded down to nearest 2MiB border
void x86_64_unmap_large_page_noflush(void* virtualaddr);

// Unmap a 2MiB page. virtualaddr will be rounded down to nearest 2MiB border
void x86_64_unmap_large_page(void* virtualaddr);

// Set the virtual and physical addresses of the kernel
void x86_64_SetKernelAddress(void* kernel_virtual, void* kernel_physical, size_t length);

// Set the HHDM start address
void x86_64_SetHHDMStart(void* virtualaddr);

extern Level4Group PML4_Array;
extern Level3Group PML3_LowestArray;
extern Level2Group PML2_LowestArray;
extern Level1Group PML1_LowestArray;
extern Level3Group PML3_KernelGroup; // only highest 2 entries are used
extern Level2Group PML2_KernelLower;
extern Level1Group PML1_KernelLowest;

#endif /* _KERNEL_X86_64_PAGE_MAP_INDEXER_HPP */