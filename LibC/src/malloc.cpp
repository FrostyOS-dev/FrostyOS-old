#include <stdlib.h>

#include <util.h>

#include <kernel/memory.h>

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
If it doesn't find one, it will allocate a new chunk from the kernel and return that.

When a chunk is freed, the allocator will add it to the free list.

Each chunk contains multiple blocks. Each block can vary in size, but it must be 16 byte aligned.
*/

#define MIN_SIZE 16

class HeapAllocator {
public:
    HeapAllocator();

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
            if (curr->size > size) {
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

    size_t numBytes = ALIGN_UP((size + sizeof(BlockHeader)), PAGE_SIZE);
    void* ptr = mmap(numBytes, PROT_READ_WRITE, nullptr);
    BlockHeader* header = (BlockHeader*)ptr;
    m_MetadataMem += sizeof(BlockHeader);
    header->size = numBytes - sizeof(BlockHeader);
    header->isFree = false;
    header->isStartOfChunk = true;
    if (header->size > size) {
        // Split the chunk into two blocks
        BlockHeader* next = (BlockHeader*)((uint64_t)header + sizeof(BlockHeader) + size);
        next->size = header->size - size - sizeof(BlockHeader);
        next->isFree = true;
        header->size = size;
        m_freeMem += next->size;
        AddToFreeList(next);
        m_MetadataMem += sizeof(BlockHeader);
    }
    m_usedMem += size;
    return (void*)((uint64_t)header + sizeof(BlockHeader));
}

void HeapAllocator::free(void* ptr) {
    BlockHeader* header = (BlockHeader*)((uint64_t)ptr - sizeof(BlockHeader));
    header->isFree = true;
    m_usedMem -= header->size;
    m_freeMem += header->size;
    AddToFreeList(header);
    CheckForDeletion();
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
    if (m_freeList == nullptr)
        m_freeList = header;
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

void HeapAllocator::CheckForDeletion() {
    if (m_freeList == nullptr)
        return;
    // search through the free list to find chunks that can be deleted
    BlockHeader* prev = nullptr;
    BlockHeader* curr = m_freeList;
    BlockHeader* next;
    while (curr != nullptr) {
        next = curr->next;
        if (next != nullptr && curr->isStartOfChunk && next->isStartOfChunk) {
            // this means that curr is an entire chunk, so we can unmap it
            m_MetadataMem -= sizeof(BlockHeader);
            m_freeMem -= curr->size;
            munmap(curr, ALIGN_UP(curr->size + sizeof(BlockHeader), PAGE_SIZE));
            curr = next;
            if (prev == nullptr) // this is the first chunk in the free list
                m_freeList = curr;
            else // this is not the first chunk in the free list
                prev->next = curr;
        }
        prev = curr;
        curr = curr->next;
    }
}

HeapAllocator g_heapAllocator;

void* malloc(size_t size) {
    return g_heapAllocator.allocate(size);
}

void free(void* ptr) {
    g_heapAllocator.free(ptr);
}

uint64_t get_heap_size() {
    return g_heapAllocator.getTotalMem();
}