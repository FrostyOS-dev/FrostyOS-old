#include "wos-stdint.h"
#include "wos-stddef.h"
#include "kernel.h"
#include "Memory.h"
#include "limine.h"

extern "C" volatile struct limine_framebuffer_request framebuffer_request {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

extern "C" volatile struct limine_memmap_request memmap_request {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};
 
static volatile void done(void) {
    while (true) {
        __asm__("hlt");
    }
}

/*static WorldOS::MemoryMapEntry operator=(limine_memmap_entry entry) {

}*/

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

    limine_memmap_response* memmap_response = memmap_request.response;
    
    limine_framebuffer* buffer = framebuffer_response->framebuffers[0];

    WorldOS::MemoryMapEntry* memoryMap = (WorldOS::MemoryMapEntry*)*memmap_response->entries;

    /*for (uint64_t i = 0; i < memmap_response->entry_count; i++) {
        limine_memmap_entry entry = *memmap_response->entries[i];
        memoryMap[i] = {entry.base, entry.length, entry.type};
    }*/

    WorldOS::InitKernel(buffer->address, buffer->width, buffer->height, (WorldOS::MemoryMapEntry**)&memoryMap, memmap_response->entry_count);
 
    // We're done, just hang...
    done();
}