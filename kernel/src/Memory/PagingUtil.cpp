#include "PagingUtil.hpp"

#include <arch/x86_64/Memory/PageMapIndexer.hpp>

void MapPage(void* phys_addr, void* virt_addr, uint32_t flags) {
    x86_64_map_page(phys_addr, virt_addr, flags);
}

void UnmapPage(void* virt_addr) {
    x86_64_unmap_page(virt_addr);
}
