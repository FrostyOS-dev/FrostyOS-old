/*
Copyright (©) 2022-2024  Frosty515

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "kmalloc.hpp"
#include "newdelete.hpp"

#include <stdint.h>
#include <stddef.h>

#include <util.h>

#include <Memory/PageManager.hpp>

#include <HAL/hal.hpp>

struct BlockHeader {
    size_t size : 62;
    size_t isFree : 1;
    size_t isStartOfChunk : 1;
    BlockHeader* next;
} __attribute__((packed));

/*
Each chunk of memory allocated by the allocator will have a header,
which will contain the size of the chunk, whether it is free or not, and a pointer to the next chunk.

The allocator will keep a pointer to the first chunk in the free list.

When a chunk is allocated, the allocator will search the free list for a chunk that is large enough.
If it finds one, it will remove it from the free list and return it.

When a chunk is freed, the allocator will add it to the free list.

Each chunk contains multiple blocks. Each block can vary in size, but it must be 16 byte aligned.
*/

#define MIN_SIZE 16

class HeapAllocator {
public:
    HeapAllocator();
    HeapAllocator(size_t size);

    void* allocate(size_t size);
    void free(void* ptr);

    size_t getFreeMem() const;
    size_t getUsedMem() const;
    size_t getMetadataMem() const;
    size_t getTotalMem() const;

private:
    void AddToFreeList(BlockHeader* header);
    void CheckForDeletion(); // will check if any chunks in the free list can be deleted, and delete them if they can

private:
    BlockHeader* m_freeList;
    size_t m_freeMem;
    size_t m_usedMem;
    size_t m_MetadataMem;
};

HeapAllocator::HeapAllocator() : m_freeList(nullptr), m_freeMem(0), m_usedMem(0), m_MetadataMem(0) {
    
}

HeapAllocator::HeapAllocator(size_t size) : m_freeList(nullptr), m_freeMem(0), m_usedMem(0), m_MetadataMem(0) {
    size_t numPages = DIV_ROUNDUP((size + sizeof(BlockHeader)), PAGE_SIZE);
    size_t numBytes = numPages * PAGE_SIZE;
    void* ptr = g_KPM->AllocatePages(numPages);
    BlockHeader* header = (BlockHeader*)ptr;
    m_MetadataMem += sizeof(BlockHeader);
    header->size = numBytes - sizeof(BlockHeader);
    header->isFree = true;
    header->isStartOfChunk = true;
    header->next = nullptr;
    m_freeMem += header->size;
    m_freeList = header;
}

void* HeapAllocator::allocate(size_t size) {
    size = ALIGN_UP(size, MIN_SIZE);

    // Search the free list for a chunk that is large enough
    BlockHeader* prev = nullptr;
    BlockHeader* curr = m_freeList;
    while (curr != nullptr) {
        if (curr->size >= size) {
            // Found a chunk that is large enough
            if (prev == nullptr) // This is the first chunk in the free list
                m_freeList = curr->next;
            else // This is not the first chunk in the free list
                prev->next = curr->next;
            curr->isFree = false;
            if (curr->size > (size + sizeof(BlockHeader))) {
                // Split the chunk into two blocks
                BlockHeader* next = (BlockHeader*)((uint64_t)curr + sizeof(BlockHeader) + size);
                next->size = curr->size - size - sizeof(BlockHeader);
                next->isFree = true;
                curr->size = size;
                m_freeMem -= sizeof(BlockHeader);
                AddToFreeList(next);
                m_MetadataMem += sizeof(BlockHeader);
            }
            m_usedMem += size;
            m_freeMem -= size;
            return (void*)((uint64_t)curr + sizeof(BlockHeader));
        }
        prev = curr;
        curr = curr->next;
    }

    // Didn't find a chunk that is large enough, and kernel heap expansion is not supported, so we just panic
    PANIC("kmalloc: out of memory");
    return nullptr;
}

void HeapAllocator::free(void* ptr) {
    BlockHeader* header = (BlockHeader*)((uint64_t)ptr - sizeof(BlockHeader));
    header->isFree = true;
    m_usedMem -= header->size;
    m_freeMem += header->size;
    AddToFreeList(header);
}

size_t HeapAllocator::getFreeMem() const {
    return m_freeMem;
}

size_t HeapAllocator::getUsedMem() const {
    return m_usedMem;
}

size_t HeapAllocator::getMetadataMem() const {
    return m_MetadataMem;
}

size_t HeapAllocator::getTotalMem() const {
    return m_usedMem + m_freeMem + m_MetadataMem;
}

void HeapAllocator::AddToFreeList(BlockHeader* header) {
    if (m_freeList == nullptr) {
        m_freeList = header;
        header->next = nullptr;
        return;
    }
    // search through the list to find the correct place to insert the chunk
    BlockHeader* prev = nullptr;
    BlockHeader* curr = m_freeList;
    while (curr != nullptr) {
        if (header > prev && header < curr) {
            // found the correct place to insert the chunk
            if (prev == nullptr) {
                // this is the first chunk in the free list
                m_freeList = header;
                // see if we can merge with next. this will only happen if the next chunk is free
                if ((uint64_t)header + sizeof(BlockHeader) + header->size == (uint64_t)curr && curr->isFree) {
                    header->size += sizeof(BlockHeader) + curr->size;
                    header->next = curr->next;
                    m_freeMem += sizeof(BlockHeader);
                    m_MetadataMem -= sizeof(BlockHeader);
                } 
                else
                    header->next = curr;

            }
            else {
                // this is not the first chunk in the free list

                // see if we can merge with prev. this will only happen if the prev chunk is free
                if ((uint64_t)prev + sizeof(BlockHeader) + prev->size == (uint64_t)header && prev->isFree) {
                    prev->size += sizeof(BlockHeader) + header->size;
                    prev->next = curr;
                    header = prev;
                    m_freeMem += sizeof(BlockHeader);
                    m_MetadataMem -= sizeof(BlockHeader);
                }
                else
                    prev->next = header;
                // see if we can merge with next. this will only happen if the next chunk is free
                if ((uint64_t)header + sizeof(BlockHeader) + header->size == (uint64_t)curr && curr->isFree) {
                    header->size += sizeof(BlockHeader) + curr->size;
                    header->next = curr->next;
                    m_freeMem += sizeof(BlockHeader);
                    m_MetadataMem -= sizeof(BlockHeader);
                }
                else
                    header->next = curr;
            }
            return;
        }
        prev = curr;
        curr = curr->next;
    }

    // didn't find the correct place to insert the chunk, so we just insert it at the end
    prev->next = header;
    header->next = nullptr;
    // see if we can merge with prev. this will only happen if the prev chunk is free
    if ((uint64_t)prev + sizeof(BlockHeader) + prev->size == (uint64_t)header && prev->isFree) {
        prev->size += sizeof(BlockHeader) + header->size;
        prev->next = nullptr;
        header = prev;
        m_freeMem += sizeof(BlockHeader);
        m_MetadataMem -= sizeof(BlockHeader);
    }
}

HeapAllocator g_heapAllocator;

bool g_kmalloc_initialised;

void kmalloc_init() {
    g_kmalloc_initialised = false;
    
    g_heapAllocator = HeapAllocator(MiB(4)); // initialise the heap with 4MiB of memory

    g_kmalloc_initialised = true;
    NewDeleteInit();
}

extern "C" void* kcalloc(size_t num, size_t size) {
    if (!g_kmalloc_initialised)
        return nullptr;
    size = ALIGN_UP((size * num), MIN_SIZE);
    void* mem = g_heapAllocator.allocate(size);
    if (mem == nullptr)
        return nullptr;
    fast_memset(mem, 0, size / 8);
    return mem;
}

extern "C" void kfree(void* addr) {
    return g_heapAllocator.free(addr);
}

extern "C" void* kmalloc(size_t size) {
    if (size == 0 || !g_kmalloc_initialised)
        return nullptr;
    return g_heapAllocator.allocate(ALIGN_UP(size, MIN_SIZE));
}

extern "C" void* krealloc(void* ptr, size_t size) {
    if (size == 0) {
        kfree(ptr);
        return nullptr;
    }
    if (ptr == nullptr)
        return kmalloc(size);
    /*if (ptr < g_mem_start || ptr > g_mem_end)
        return nullptr;
    size = ALIGN_UP(size, MIN_SIZE);*/
    BlockHeader* header = (BlockHeader*)((uint64_t)ptr - sizeof(BlockHeader));
    size_t old_size = header->size;
    void* ptr2 = kmalloc(size);
    if (ptr2 == nullptr)
        return nullptr;
    fast_memcpy(ptr2, ptr, old_size);
    kfree(ptr);
    return ptr2;
}


bool g_kmalloc_eternal_initialised = false;

void* g_kmalloc_eternal_mem = nullptr;
size_t g_kmalloc_eternal_free_mem = 0;
size_t g_kmalloc_eternal_used_mem = 0;

void kmalloc_eternal_init() {
    g_kmalloc_eternal_initialised = false;

    void* pages = g_KPM->AllocatePages(512); // ~2MiB
    if (pages == nullptr)
        return;
    g_kmalloc_eternal_mem = pages;
    g_kmalloc_eternal_free_mem = 512 * PAGE_SIZE;
    g_kmalloc_eternal_used_mem = 0;

    g_kmalloc_eternal_initialised = true;
}

extern "C" void* kmalloc_eternal(size_t size) {
    size = ALIGN_UP(size, 8);

    if (!g_kmalloc_eternal_initialised || size > g_kmalloc_eternal_free_mem)
        return nullptr;

    void* mem = g_kmalloc_eternal_mem;
    g_kmalloc_eternal_used_mem += size;
    g_kmalloc_eternal_free_mem -= size;
    g_kmalloc_eternal_mem = (void*)((uint64_t)g_kmalloc_eternal_mem + size);

    return mem;
}

extern "C" void* kcalloc_eternal(size_t num, size_t size) {
    size = ALIGN_UP(size * num, 8);

    if (!g_kmalloc_eternal_initialised || size > g_kmalloc_eternal_free_mem)
        return nullptr;

    void* mem = g_kmalloc_eternal_mem;
    g_kmalloc_eternal_used_mem += size;
    g_kmalloc_eternal_free_mem -= size;
    g_kmalloc_eternal_mem = (void*)((uint64_t)g_kmalloc_eternal_mem + size);

    fast_memset(mem, 0, size / 8);

    return mem;
}
