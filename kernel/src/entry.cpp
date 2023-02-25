#include <stdint.h>
#include <stddef.h>

#include "kernel.hpp"
#include "Memory/Memory.hpp"
#include "limine.h"

#include <arch/x86_64/Stack.h>

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

extern "C" volatile struct limine_kernel_file_request kernel_file_request {
    .id = LIMINE_KERNEL_FILE_REQUEST,
    .revision = 0
};
 
static volatile void done(void) {
    while (true) {
        asm("hlt");
    }
}

extern "C" void _start(void) {
    InitKernelStack(kernel_stack, KERNEL_STACK_SIZE);

    if (framebuffer_request.response == nullptr) {
        done();
    }

    limine_framebuffer_response* framebuffer_response = framebuffer_request.response;

    if (!(framebuffer_response->framebuffer_count > 0)) {
        done();
    }

    if (memmap_request.response == nullptr) {
        done();
    }

    if (efi_system_table_request.response == nullptr) {
        done();
    }

    if (rsdp_request.response == nullptr) {
        done();
    }

    if (kernel_address_request.response == nullptr) {
        done();
    }

    if (kernel_file_request.response == nullptr) {
        done();
    }

    limine_kernel_file_response* kernel_file_response = kernel_file_request.response;

    if (kernel_file_response->kernel_file == nullptr) {
        done();
    }

    limine_efi_system_table_response* efi_system_table_response = efi_system_table_request.response;

    limine_rsdp_response* rsdp_response = rsdp_request.response;

    limine_kernel_address_response* kernel_address_response = kernel_address_request.response;

    limine_memmap_response* memmap_response = memmap_request.response;
    
    limine_framebuffer* buffer = framebuffer_response->framebuffers[0];

    limine_file* kernel_file = kernel_file_response->kernel_file;

    if (kernel_file->size == 0) {
        done();
    }

    WorldOS::MemoryMapEntry** memoryMap = (WorldOS::MemoryMapEntry**)memmap_response->entries;

    WorldOS::KernelParams kernelParams = {
        .frameBuffer = {buffer->address, buffer->width, buffer->height, buffer->bpp},
        .MemoryMap = memoryMap,
        .MemoryMapEntryCount = memmap_response->entry_count,
        .EFI_SYSTEM_TABLE_ADDR = efi_system_table_response->address,
        .kernel_physical_addr = kernel_address_response->physical_base,
        .kernel_virtual_addr = kernel_address_response->virtual_base,
        .kernel_size = kernel_file->size,
        .RSDP_table = rsdp_response->address
    };

    StartKernel(kernelParams);
 
    // We're done, just hang...
    done();
}
