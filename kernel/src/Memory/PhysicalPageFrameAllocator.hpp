/*
Copyright (©) 2022-2023  Frosty515

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

        // Early initialise the first 4GiB using builtin bitmap
        void EarlyInit(const MemoryMapEntry* FirstMemoryMapEntry, const size_t MemoryMapEntryCount, uint64_t MemorySize);

        // Fully initialise with full memory map and bitmap
        void FullInit(const MemoryMapEntry* FirstMemoryMapEntry, const size_t MemoryMapEntryCount, uint64_t MemorySize);

        void* AllocatePage(); // Only for physical allocation, DO NOT use for virtual allocation.
        void* AllocatePages(uint64_t count);

        void ReservePage(void* page);
        void ReservePages(void* start, uint64_t count);
        void UnreservePage(void* page);
        void UnreservePages(void* start, uint64_t count);

        void FreePage(void* page);
        void FreePages(void* start, uint64_t count);

        inline size_t GetFreeMemory()     { return m_FreeMem;     };
        inline size_t GetUsedMemory()     { return m_UsedMem;     };
        inline size_t GetReservedMemory() { return m_ReservedMem; };

    private:
        void LockPage(void* page);
        void LockPages(void* start, uint64_t count);
        void UnlockPage(void* page);
        void UnlockPages(void* start, uint64_t count);
        uint64_t FindFreePage();
        uint64_t FindFreePages(uint64_t count);
    
    private:
        Bitmap m_Bitmap;
        size_t m_FreeMem;
        size_t m_ReservedMem;
        size_t m_UsedMem;
        size_t m_MemSize;
    };

}

extern WorldOS::PhysicalPageFrameAllocator* g_PPFA;
extern uint8_t g_EarlyBitmap[128 * 1024];

#endif /* _KERNEL_PHYSICAL_PAGE_FRAME_ALLOCATOR_HPP */