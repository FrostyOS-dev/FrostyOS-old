#ifndef _KERNEL_PAGE_FRAME_ALLOCATOR_HPP
#define _KERNEL_PAGE_FRAME_ALLOCATOR_HPP

#include <stddef.h>
#include <stdint.h>
#include <Data-structures/Bitmap.hpp>

#include <Memory/Memory.hpp>

#define MEMORY_MAP_ENTRY_SIZE 24

namespace x86_64_WorldOS {

    class PageFrameAllocator {
    public:
        PageFrameAllocator();
        ~PageFrameAllocator();

        // Set Memory Map and Initialize Page map
        void SetMemoryMap(const WorldOS::MemoryMapEntry* FirstMemoryMapEntry, const size_t MemoryMapEntryCount);

        void* AllocatePage(); // Only for physical allocation, DO NOT use for virtual allocation.

        void  ReservePage(void* page);
        void  FreePage(void* page);

        inline size_t GetFreeMemory()     { return m_FreeMem;     };
        inline size_t GetUsedMemory()     { return m_UsedMem;     };
        inline size_t GetReservedMemory() { return m_ReservedMem; };

    private:
        void LockPage(void* page);
        void UnlockPage(void* page);
        uint64_t FindFreePage();
    
    private:
        WorldOS::Bitmap m_Bitmap;
        size_t m_FreeMem;
        size_t m_ReservedMem;
        size_t m_UsedMem;
        size_t m_MemSize;
    };

}

extern x86_64_WorldOS::PageFrameAllocator* g_PFA;

#endif /* _KERNEL_PAGE_FRAME_ALLOCATOR_HPP */