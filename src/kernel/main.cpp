#include "stdint.h"
#include "stddef.h"
#include "kernel.h"
#include "Memory/Memory.h"
#include "limine.h"

extern "C" volatile struct limine_framebuffer_request framebuffer_request {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

extern "C" volatile struct limine_memmap_request memmap_request {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

extern "C" volatile struct limine_efi_system_table_request efi_system_table_request {
    .id = LIMINE_EFI_SYSTEM_TABLE_REQUEST,
    .revision = 0
};

extern "C" volatile struct limine_rsdp_request rsdp_request {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

extern "C" volatile struct limine_kernel_address_request kernel_address_request {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};
 
static volatile void done(void) {
    while (true) {
        asm("hlt");
    }
}

extern "C" void _start(void) {

    if (framebuffer_request.response == NULL) {
        done();
    }

    limine_framebuffer_response* framebuffer_response = framebuffer_request.response;

    if (!(framebuffer_response->framebuffer_count > 0)) {
        done();
    }

    if (memmap_request.response == NULL) {
        done();
    }

    if (efi_system_table_request.response == NULL) {
        done();
    }

    if (rsdp_request.response == NULL) {
        done();
    }

    if (kernel_address_request.response == NULL) {
        done();
    }

    limine_efi_system_table_response* efi_system_table_response = efi_system_table_request.response;

    limine_rsdp_response* rsdp_response = rsdp_request.response;

    limine_kernel_address_response* kernel_address_response = kernel_address_request.response;

    limine_memmap_response* memmap_response = memmap_request.response;
    
    limine_framebuffer* buffer = framebuffer_response->framebuffers[0];

    WorldOS::MemoryMapEntry** memoryMap = (WorldOS::MemoryMapEntry**)memmap_response->entries;

    WorldOS::KernelParams kernelParams = {
        .frameBuffer = {buffer->address, buffer->width, buffer->height, buffer->bpp},
        .MemoryMap = memoryMap,
        .MemoryMapEntryCount = memmap_response->entry_count,
        .EFI_SYSTEM_TABLE_ADDR = efi_system_table_response->address,
        .kernel_physical_addr = kernel_address_response->physical_base,
        .kernel_virtual_addr = kernel_address_response->virtual_base,
        .RSDP_table = rsdp_response->address
    };

    WorldOS::Kernel kernel(kernelParams);
 
    // We're done, just hang...
    done();
}