#ifndef _KERNEL_PHYSICAL_PAGE_FRAME_ALLOCATOR_HPP
#define _KERNEL_PHYSICAL_PAGE_FRAME_ALLOCATOR_HPP

#include <stddef.h>
#include <stdint.h>
#include <Data-structures/Bitmap.hpp>

#include <Memory/Memory.hpp>


namespace WorldOS {

    class PhysicalPageFrameAllocator {
    public:
        PhysicalPageFrameAllocator();
        ~PhysicalPageFrameAllocator();

        // Set Memory Map and Initialize Page map
        void SetMemoryMap(const MemoryMapEntry* FirstMemoryMapEntry, const size_t MemoryMapEntryCount);

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
        Bitmap m_Bitmap;
        size_t m_FreeMem;
        size_t m_ReservedMem;
        size_t m_UsedMem;
        size_t m_MemSize;
    };

}

extern WorldOS::PhysicalPageFrameAllocator* g_PPFA;

#endif /* _KERNEL_PHYSICAL_PAGE_FRAME_ALLOCATOR_HPP */