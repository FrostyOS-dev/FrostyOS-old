#ifndef _PAGING_UTIL_H
#define _PAGING_UTIL_H

#include <stdint.h>

void MapPage(void* phys_addr, void* virt_addr, uint32_t flags);
void UnmapPage(void* virt_addr);

void* to_HHDM(void* phys_addr);

void* get_physaddr(void* virtaddr);

#endif /* _PAGING_UTIL_H */