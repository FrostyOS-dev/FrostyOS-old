#include "PageFrameAllocator.h"

namespace WorldOS {

    /* PageFrameAllocator class */

    /* Public Methods */

    PageFrameAllocator::PageFrameAllocator(const MemoryMapEntry* FirstMemoryMapEntry, const size_t MemoryMapEntryCount) {
        m_MemSize = GetMemorySize(&FirstMemoryMapEntry, MemoryMapEntryCount);
        m_FreeMem = 0;
        m_ReservedMem = 0;
        m_UsedMem = 0;
        m_Bitmap.Size = (m_MemSize / 4096 / 8) + 1;
        bool BitmapAddressFound = false;
        for (uint64_t i = 0; i < MemoryMapEntryCount; i++) {
            MemoryMapEntry* entry = (MemoryMapEntry*)((uint64_t)FirstMemoryMapEntry + (i * MEMORY_MAP_ENTRY_SIZE));
            if ((entry->type == WORLDOS_MEMORY_FREE) && (entry->length >= m_Bitmap.Size)) {
                m_ReservedMem += m_Bitmap.Size;
                m_Bitmap.Buffer = (uint8_t*)entry->Address;
                for (uint64_t j = 0; j < m_Bitmap.Size; j+=8) {
                    *(uint64_t*)(m_Bitmap.Buffer + (j*8)) = 0;
                }
                m_Bitmap.Set(entry->Address / 4096, true);
                BitmapAddressFound = true;
            }
        }
        if (!BitmapAddressFound) int i = 1/0; /* creates exception */
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


    }

    void* PageFrameAllocator::AllocatePage() {
        uint64_t index = FindFreePage();
        if (index == 0) return NULL;
        m_Bitmap.Set(index, true);
        m_FreeMem -= 4096;
        m_UsedMem += 4096;

        return (void*)(index * 4096);
    }

    void PageFrameAllocator::FreePage(void* page) {
        if (m_Bitmap[((uint64_t)page / 4096)] == false) return;
        m_Bitmap.Set(((uint64_t)page / 4096), false);
        m_FreeMem += 4096;
        m_UsedMem -= 4096;
    }

    /* Private Methods */

    uint64_t PageFrameAllocator::FindFreePage() {
        for (uint64_t i = 0; i < m_Bitmap.Size; i++) {
            if (m_Bitmap[i] == false) {
                return i * 8;
            }
        }
        return 0;
    }

}