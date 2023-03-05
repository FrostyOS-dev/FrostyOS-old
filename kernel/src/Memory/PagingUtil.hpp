#ifndef _PAGING_UTIL_H
#define _PAGING_UTIL_H

#include <stdint.h>

void MapPage(void* phys_addr, void* virt_addr, uint32_t flags);

#endif /* _PAGING_UTIL_H */