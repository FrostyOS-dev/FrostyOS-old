#ifndef _KERNEL_PAGE_FRAME_ALLOCATOR_H
#define _KERNEL_PAGE_FRAME_ALLOCATOR_H

#include "wos-stddef.h"
#include "wos-stdint.h"

#include "Memory.h"
#include "Bitmap.h"

#define MEMORY_MAP_ENTRY_SIZE 24

namespace WorldOS {

    class PageFrameAllocator {
    public:
        PageFrameAllocator(MemoryMapEntry* FirstMemoryMapEntry, const size_t MemoryMapEntryCount);

        void* AllocatePage();
        void  FreePage(void* page);

        inline size_t GetFreeMemory()     { return m_FreeMem;     };
        inline size_t GetUsedMemory()     { return m_UsedMem;     };
        inline size_t GetReservedMemory() { return m_ReservedMem; };

    private:

        uint64_t FindFreePage();
    
    private:
        Bitmap m_Bitmap;
        size_t m_FreeMem;
        size_t m_ReservedMem;
        size_t m_UsedMem;
        size_t m_MemSize;
    };

}

#endif /* _KERNEL_PAGE_FRAME_ALLOCATOR_H */