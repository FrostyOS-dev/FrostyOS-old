#include "PhysicalPageFrameAllocator.hpp"

#include <util.h>
#include <stdio.hpp>


#include <HAL/hal.hpp>

#include "PagingUtil.hpp"

WorldOS::PhysicalPageFrameAllocator* g_PPFA = nullptr;

namespace WorldOS {

    /* PhysicalPageFrameAllocator class */

    /* Public Methods */

    PhysicalPageFrameAllocator::PhysicalPageFrameAllocator() {
        m_FreeMem = 0;
        m_ReservedMem = 0;
        m_UsedMem = 0;
        m_MemSize = 0;
    }

    PhysicalPageFrameAllocator::~PhysicalPageFrameAllocator() {
        
    }

    void PhysicalPageFrameAllocator::SetMemoryMap(const MemoryMapEntry* FirstMemoryMapEntry, const size_t MemoryMapEntryCount) {
        using namespace WorldOS;
        // Setup
        m_MemSize = GetMemorySize(&FirstMemoryMapEntry, MemoryMapEntryCount);
        m_FreeMem = 0;
        m_ReservedMem = 0;
        m_UsedMem = 0;

        size_t BitmapSize = m_MemSize / 4096 / 8;
        if ((m_MemSize / 4096) % 8 != 0) {
            BitmapSize++;
        }

        m_Bitmap.SetSize(BitmapSize);
        // Get Location for Bitmap
        bool BitmapAddressFound = false;
        for (uint64_t i = 0; i < MemoryMapEntryCount; i++) {
            MemoryMapEntry* entry = (MemoryMapEntry*)((uint64_t)FirstMemoryMapEntry + (i * MEMORY_MAP_ENTRY_SIZE));
            if ((entry->Address + entry->length) > GiB(1)) break; // bitmap must be under 1 GiB
            if ((entry->type == WORLDOS_MEMORY_FREE) && (entry->length >= BitmapSize)) {
                m_ReservedMem += BitmapSize;
                m_Bitmap.SetBuffer((uint8_t*)entry->Address);
                MapPage(m_Bitmap.GetBuffer(), m_Bitmap.GetBuffer(), 3);
                fast_memset(m_Bitmap.GetBuffer(), 0, BitmapSize/8);
                for (uint64_t j = 0; j < (BitmapSize / 4096); j++) {
                    m_Bitmap.Set(((entry->Address / 4096) + j), true);
                }
                BitmapAddressFound = true;
            }
        }
        if (!BitmapAddressFound) {
            fprintf(VFS_DEBUG, "Bitmap error!\n");
            Panic("Failed to find address for page bitmap", nullptr, false);
            return;
        }
        // Fill bitmap
        for (uint64_t i = 0; i < MemoryMapEntryCount; i++) {
            MemoryMapEntry* entry = (MemoryMapEntry*)((uint64_t)FirstMemoryMapEntry + (i * MEMORY_MAP_ENTRY_SIZE));
            if (entry->type == WORLDOS_MEMORY_FREE) {
                m_FreeMem += entry->length;
                if (entry->length > 4096) {
                    for (uint64_t j = 0; j < (entry->length / 4096); j += 4096) {
                        m_Bitmap.Set(j + entry->Address, false);
                    }
                } else {
                    m_Bitmap.Set(entry->Address, false);
                }
            } else {
                m_ReservedMem += entry->length;
                if (entry->length > 4096) {
                    for (uint64_t j = 0; j < (entry->length / 4096); j += 4096) {
                        m_Bitmap.Set(j + entry->Address, true);
                    }
                } else {
                    m_Bitmap.Set(entry->Address, true);
                }
            }
        }

        if (!m_Bitmap[0]) {
            m_ReservedMem += 0x1000;
            m_Bitmap.Set(0, true);
        }
    }

    void* PhysicalPageFrameAllocator::AllocatePage() {
        uint64_t index = FindFreePage();
        LockPage((void*)(index * 4096));
        return (void*)(index * 4096);
    }

    void* PhysicalPageFrameAllocator::AllocatePages(uint64_t count) {
        uint64_t index = FindFreePages(count);
        LockPages((void*)(index * 4096), count);
        return (void*)(index * 4096);
    }

    void PhysicalPageFrameAllocator::ReservePage(void* page) {
        if (m_Bitmap[((uint64_t)page / 4096)]) return;
        m_Bitmap.Set(((uint64_t)page / 4096), true);
        m_FreeMem -= 4096;
        m_ReservedMem += 4096;
    }

    void PhysicalPageFrameAllocator::ReservePages(void* start, uint64_t count) {
        for (uint64_t i = 0; i < count; i++) {
            ReservePage((void*)(i * 4096 + (uint64_t)start));
        }
    }

    void PhysicalPageFrameAllocator::UnreservePage(void* page) {
        if (!m_Bitmap[((uint64_t)page / 4096)]) return;
        m_Bitmap.Set(((uint64_t)page / 4096), false);
        m_FreeMem += 4096;
        m_ReservedMem -= 4096;
    }

    void PhysicalPageFrameAllocator::UnreservePages(void* start, uint64_t count) {
        for (uint64_t i = 0; i < count; i++) {
            UnreservePage((void*)(i * 4096 + (uint64_t)start));
        }
    }

    void PhysicalPageFrameAllocator::FreePage(void* page) {
        if (!m_Bitmap[((uint64_t)page / 4096)]) return;
        m_Bitmap.Set(((uint64_t)page / 4096), false);
        m_FreeMem += 4096;
        m_UsedMem -= 4096;
    }

    void PhysicalPageFrameAllocator::FreePages(void* start, uint64_t count) {
        for (uint64_t i = 0; i < count; i++) {
            FreePage((void*)(i * 4096 + (uint64_t)start));
        }
    }

    /* Private Methods */

    void PhysicalPageFrameAllocator::LockPage(void* page) {
        if (m_Bitmap[((uint64_t)page / 4096)]) return;
        m_Bitmap.Set(((uint64_t)page / 4096), true);
        m_FreeMem -= 4096;
        m_UsedMem += 4096;
    }

    void PhysicalPageFrameAllocator::LockPages(void* start, uint64_t count) {
        for (uint64_t i = 0; i < count; i++) {
            LockPage((void*)(i * 4096 + (uint64_t)start));
        }
    }


    void PhysicalPageFrameAllocator::UnlockPage(void* page) {
        if (!m_Bitmap[((uint64_t)page / 4096)]) return;
        m_Bitmap.Set(((uint64_t)page / 4096), false);
        m_FreeMem += 4096;
        m_UsedMem -= 4096;
    }

    void PhysicalPageFrameAllocator::UnlockPages(void* start, uint64_t count) {
        for (uint64_t i = 0; i < count; i++) {
            UnlockPage((void*)(i * 4096 + (uint64_t)start));
        }
    }

    uint64_t PhysicalPageFrameAllocator::FindFreePage() {
        for (uint64_t i = 0; i < m_Bitmap.GetSize(); i++) {
            if (m_Bitmap[i] == 0) {
                return i * 8;
            }
        }
        return 0;
    }

    uint64_t PhysicalPageFrameAllocator::FindFreePages(uint64_t count) {
        uint64_t next = 0;
        if (count == 0) return 0;
        for (uint64_t i = 0; i < m_Bitmap.GetSize(); i+=next) {
            next = 1;
            if (m_Bitmap[i] == 0) {
                if (count == 1) return i * 8;
                bool found = true;
                next = 1;
                while (next <= count) {
                    if (m_Bitmap[i + next] == 0) {
                        found = false;
                        break;
                    }
                    if (count <= next) break; // check if next > count on next iteration
                    next++;
                }
                if (found) return (i - next) * 8; // return start page index of the block
            }
        }
        return 0;
    }

}