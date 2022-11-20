#ifndef _KERNEL_X86_64_PAGE_MAP_INDEXER_H
#define _KERNEL_X86_64_PAGE_MAP_INDEXER_H

#include "PageTables.h"
#include "PagingUtil.h"

#include <stdint.h>

void* x86_64_get_physaddr(void* virtualaddr);
void x86_64_map_page(void* physaddr, void* virtualaddr, uint32_t flags);

extern Level3Group PML3_Array;

#endif /* _KERNEL_X86_64_PAGE_MAP_INDEXER_H */