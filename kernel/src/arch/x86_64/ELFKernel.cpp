#include "ELFKernel.hpp"

#include "Memory/PageMapIndexer.hpp"

namespace x86_64_WorldOS {
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

        // pre-map kernel as Read-only and No execute in case there are more unused sections

        for (uint64_t i = (uint64_t)kernel_virt; i < kernel_size; i+=4096) {
            uint64_t phys_addr = i - (uint64_t)kernel_virt + (uint64_t)kernel_phys;
            x86_64_map_page_noflush((void*)phys_addr, (void*)i, 0x8000001);
        }

        // actually do the mapping
        for (uint64_t i = text_start_addr; i < text_size; i+=4096) {
            uint64_t phys_addr = i - (uint64_t)kernel_virt + (uint64_t)kernel_phys;
            x86_64_map_page_noflush((void*)phys_addr, (void*)i, 0x1); // Present, Read-only, Execute
        }

        for (uint64_t i = rodata_start_addr; i < rodata_size; i+=4096) {
            uint64_t phys_addr = i - (uint64_t)kernel_virt + (uint64_t)kernel_phys;
            x86_64_map_page_noflush((void*)phys_addr, (void*)i, 0x8000001); // Present, Read-only, Execute Disable
        }

        for (uint64_t i = data_start_addr; i < data_size; i+=4096) {
            uint64_t phys_addr = i - (uint64_t)kernel_virt + (uint64_t)kernel_phys;
            x86_64_map_page_noflush((void*)phys_addr, (void*)i, 0x8000003); // Present, Read-Write, Execute Disable
        }

        for (uint64_t i = bss_start_addr; i < bss_size; i+=4096) {
            uint64_t phys_addr = i - (uint64_t)kernel_virt + (uint64_t)kernel_phys;
            x86_64_map_page_noflush((void*)phys_addr, (void*)i, 0x8000003); // Present, Read-Write, Execute Disable
        }

        return true;
    }
}