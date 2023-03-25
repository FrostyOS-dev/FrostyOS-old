#ifndef _KERNEL_X86_64_PAGE_MAP_INDEXER_HPP
#define _KERNEL_X86_64_PAGE_MAP_INDEXER_HPP

#include "PageTables.hpp"
#include "PagingUtil.hpp"

#include <stdint.h>
#include <stddef.h>

// Get the page-aligned physical address from a page-aligned virtual address.
void* x86_64_get_physaddr(void* virtualaddr);

// Map a page. physaddr and virtualaddr must be page aligned. Can potentially map 512 pages instead of just 1 due to x86_64 limitations.
void x86_64_map_page(void* physaddr, void* virtualaddr, uint32_t flags);

// Map a page with no TLB flush. physaddr and virtualaddr must be page aligned. Can potentially map 512 pages instead of just 1 due to x86_64 limitations.
void x86_64_map_page_noflush(void* physaddr, void* virtualaddr, uint32_t flags);

// Identity map memory. If length and/or start_phys aren't page aligned, the values used are rounded down to the nearest page boundary.
void x86_64_identity_map(void* start_phys, uint64_t length, uint32_t flags);

// Set the virtual and physical addresses of the kernel
void x86_64_SetKernelAddress(void* kernel_virtual, void* kernel_physical, size_t length);

extern Level4Group PML4_Array;
extern Level3Group PML3_LowestArray;
extern Level2Group PML2_LowestArray;
extern Level1Group PML1_LowestArray;
extern Level3Group PML3_KernelGroup; // only highest 2 entries are used
extern Level2Group PML2_KernelLower;
extern Level1Group PML1_KernelLowest;

#endif /* _KERNEL_X86_64_PAGE_MAP_INDEXER_HPP */