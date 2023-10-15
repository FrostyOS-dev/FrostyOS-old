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

#include "PhysicalPageFrameAllocator.hpp"

#include <util.h>
#include <stdio.h>


#include <HAL/hal.hpp>

#include "PagingUtil.hpp"

WorldOS::PhysicalPageFrameAllocator* g_PPFA = nullptr;
uint8_t g_EarlyBitmap[128 * 1024] = {0};

namespace WorldOS {

    /* PhysicalPageFrameAllocator class */

    /* Public Methods */

    PhysicalPageFrameAllocator::PhysicalPageFrameAllocator() {
        m_FreeMem = 0;
        m_ReservedMem = 0;
        m_UsedMem = 0;
        m_MemSize = 0;
        m_nextFree = UINT64_MAX;
    }

    PhysicalPageFrameAllocator::~PhysicalPageFrameAllocator() {
        
    }

    void PhysicalPageFrameAllocator::EarlyInit(const MemoryMapEntry* FirstMemoryMapEntry, const size_t MemoryMapEntryCount, uint64_t MemorySize) {
        using namespace WorldOS;
        // Setup
        m_MemSize = (size_t)MemorySize;
        if (m_MemSize > GiB(4))
            m_MemSize = GiB(4);
        m_FreeMem = 0;
        m_ReservedMem = 0;
        m_UsedMem = 0;
        m_nextFree = UINT64_MAX;

        size_t BitmapSize = m_MemSize >> 15; // divide by (4096 * 8)
        if (((m_MemSize >> 12) & 7) != 0) {
            BitmapSize++;
        }
        m_Bitmap.SetSize(BitmapSize);
        m_Bitmap.SetBuffer(g_EarlyBitmap);

        // Fill bitmap
        for (uint64_t i = 0; i < MemoryMapEntryCount; i++) {
            MemoryMapEntry* entry = (MemoryMapEntry*)((uint64_t)FirstMemoryMapEntry + (i * MEMORY_MAP_ENTRY_SIZE));
            // We don't need to check if the address is in bitmap range because the bitmap has protections
            if (entry->type == WORLDOS_MEMORY_FREE) {
                m_FreeMem += entry->length;
                if (entry->length > 4096) {
                    for (uint64_t j = 0; j < entry->length; j += 4096) {
                        m_Bitmap.Set(entry->Address + j, false);
                    }
                } else {
                    m_Bitmap.Set(entry->Address, false);
                }
            } else {
                m_ReservedMem += entry->length;
                uint64_t length = entry->length;
                // round up
                if ((entry->length & 4095) != 0) {
                    length &= ~4095;
                    length += 4096;
                } 
                if (length > 4096) {
                    for (uint64_t j = 0; j < length; j += 4096) {
                        m_Bitmap.Set(j + entry->Address, true);
                    }
                } else {
                    m_Bitmap.Set(entry->Address, true);
                }
            }
        }

        if (m_Bitmap[0] == 0) {
            m_ReservedMem += 0x1000;
            m_Bitmap.Set(0, true);
        }
    }

    void PhysicalPageFrameAllocator::FullInit(const MemoryMapEntry* FirstMemoryMapEntry, const size_t MemoryMapEntryCount, uint64_t MemorySize) {
        using namespace WorldOS;
        // Setup
        m_MemSize = MemorySize;

        size_t BitmapSize = m_MemSize >> 15;
        if (((m_MemSize >> 12) & 7) != 0) {
            BitmapSize++;
        }

        m_Bitmap.SetSize(BitmapSize);
        // Get Location for Bitmap
        if (m_MemSize > GiB(4)) {
            bool BitmapAddressFound = false;
            for (uint64_t i = 0; i < MemoryMapEntryCount; i++) {
                MemoryMapEntry* entry = (MemoryMapEntry*)((uint64_t)FirstMemoryMapEntry + (i * MEMORY_MAP_ENTRY_SIZE));
                if ((entry->type == WORLDOS_MEMORY_FREE) && (entry->length >= BitmapSize)) {
                    m_ReservedMem += BitmapSize;
                    m_Bitmap.SetBuffer((uint8_t*)entry->Address);
                    MapPage(m_Bitmap.GetBuffer(), m_Bitmap.GetBuffer(), 0x8000003); // Read/Write, Present, Execute Disable
                    fast_memset(m_Bitmap.GetBuffer(), 0, BitmapSize >> 3); // Avoid divide
                    for (uint64_t j = 0; j < (BitmapSize >> 12); j++) {
                        m_Bitmap.Set(((entry->Address >> 12) + j), true);
                    }
                    BitmapAddressFound = true;
                }
            }
            if (!BitmapAddressFound) {
                dbgprintf("Bitmap error!\n");
                PANIC("Failed to find address for page bitmap");
                return;
            }
        }

        // Copy old bitmap
        if (m_Bitmap.GetBuffer() != g_EarlyBitmap)
            fast_memcpy(m_Bitmap.GetBuffer(), g_EarlyBitmap, KiB(128));

        // Fill bitmap
        for (uint64_t i = 0; i < MemoryMapEntryCount; i++) {
            MemoryMapEntry* entry = (MemoryMapEntry*)((uint64_t)FirstMemoryMapEntry + (i * MEMORY_MAP_ENTRY_SIZE));
            if ((entry->Address + entry->length) <= GiB(4))
                continue; // ignore entries below 4GiB
            if (entry->type == WORLDOS_MEMORY_FREE) {
                m_FreeMem += entry->length;
                if (entry->length > 4096) {
                    for (uint64_t j = 0; j < (entry->length >> 12 /* avoid divide as it is very slow */); j += 4096) {
                        if ((entry->Address + j) < GiB(4))
                            continue; // skip entries below 4GiB
                        m_Bitmap.Set(j + entry->Address, false);
                    }
                } else {
                    if (!(entry->Address < GiB(4)))
                        m_Bitmap.Set(entry->Address, false);
                }
            } else {
                // Address and length must be made page aligned because they might not be
                m_ReservedMem += entry->length;
                uint64_t length = entry->length;
                if ((length & 0xFFF) != 0) { // avoid divide as it is very slow
                    length &= ~0xFFF;
                    length += 0x1000;
                }
                uint64_t addr = entry->Address & ~0xFFF;
                if (length > 4096) {
                    length >>= 12; // avoid divide as it is very slow
                    for (uint64_t j = 0; j < length; j += 4096) {
                        if ((addr + j) < GiB(4))
                            continue; // skip entries below 4GiB
                        m_Bitmap.Set(j + addr, true);
                    }
                } else {
                    if (!(addr < GiB(4)))
                        m_Bitmap.Set(addr, true);
                }
            }
        }

        if (m_Bitmap[0] == 0) {
            m_ReservedMem += 0x1000;
            m_Bitmap.Set(0, true);
        }

        m_nextFree = UINT64_MAX;
    }

    void* PhysicalPageFrameAllocator::AllocatePage() {
        uint64_t index = FindFreePage();
        if (index == UINT64_MAX) { // Out of memory. Panic immediately
            PANIC("OUT OF MEMORY. No physical pages are available.");
        }
        LockPage((void*)(index * 4096));
        return (void*)(index * 4096);
    }

    void* PhysicalPageFrameAllocator::AllocatePages(uint64_t count) {
        uint64_t index = FindFreePages(count);
        if (index == UINT64_MAX)
            return nullptr;
        LockPages((void*)(index * 4096), count);
        return (void*)(index * 4096);
    }

    void PhysicalPageFrameAllocator::ReservePage(void* page) {
        if (m_Bitmap[((uint64_t)page >> 12)]) return;
        m_Bitmap.Set(((uint64_t)page >> 12), true);
        m_FreeMem -= 4096;
        m_ReservedMem += 4096;
        if (m_nextFree == ((uint64_t)page >> 12))
            m_nextFree = UINT64_MAX;
    }

    void PhysicalPageFrameAllocator::ReservePages(void* start, uint64_t count) {
        for (uint64_t i = 0; i < count; i++) {
            ReservePage((void*)(i * 4096 + (uint64_t)start));
        }
    }

    void PhysicalPageFrameAllocator::UnreservePage(void* page) {
        if (!m_Bitmap[((uint64_t)page >> 12)]) return;
        m_Bitmap.Set(((uint64_t)page >> 12), false);
        m_FreeMem += 4096;
        m_ReservedMem -= 4096;
        m_nextFree = ((uint64_t)page >> 12);
    }

    void PhysicalPageFrameAllocator::UnreservePages(void* start, uint64_t count) {
        for (uint64_t i = 0; i < count; i++) {
            UnreservePage((void*)(i * 4096 + (uint64_t)start));
        }
    }

    void PhysicalPageFrameAllocator::FreePage(void* page) {
        if (!m_Bitmap[((uint64_t)page >> 12)]) return;
        m_Bitmap.Set(((uint64_t)page >> 12), false);
        m_FreeMem += 4096;
        m_UsedMem -= 4096;
        m_nextFree = ((uint64_t)page >> 12);
    }

    void PhysicalPageFrameAllocator::FreePages(void* start, uint64_t count) {
        for (uint64_t i = 0; i < count; i++) {
            FreePage((void*)(i * 4096 + (uint64_t)start));
        }
    }

    /* Private Methods */

    void PhysicalPageFrameAllocator::LockPage(void* page) {
        if (m_Bitmap[((uint64_t)page >> 12)]) return;
        m_Bitmap.Set(((uint64_t)page >> 12), true);
        m_FreeMem -= 4096;
        m_UsedMem += 4096;
    }

    void PhysicalPageFrameAllocator::LockPages(void* start, uint64_t count) {
        for (uint64_t i = 0; i < count; i++) {
            LockPage((void*)(i * 4096 + (uint64_t)start));
        }
    }


    void PhysicalPageFrameAllocator::UnlockPage(void* page) {
        if (!m_Bitmap[((uint64_t)page >> 12)]) return;
        m_Bitmap.Set(((uint64_t)page >> 12), false);
        m_FreeMem += 4096;
        m_UsedMem -= 4096;
    }

    void PhysicalPageFrameAllocator::UnlockPages(void* start, uint64_t count) {
        for (uint64_t i = 0; i < count; i++) {
            UnlockPage((void*)(i * 4096 + (uint64_t)start));
        }
    }

    uint64_t PhysicalPageFrameAllocator::FindFreePage() {
        if (m_nextFree != UINT64_MAX) {
            uint64_t i = m_nextFree;
            if ((i + 1) < (m_Bitmap.GetSize() << 3) && m_Bitmap[i + 1] == 0)
                m_nextFree = i + 1;
            else
                m_nextFree = UINT64_MAX;
            return i;
        }
        for (uint64_t i = 0; i < (m_Bitmap.GetSize() << 3); i++) {
            if (m_Bitmap[i] == 0) {
                if ((i + 1) < (m_Bitmap.GetSize() << 3) && m_Bitmap[i + 1] == 0)
                    m_nextFree = i + 1;
                return i;
            }
        }
        return UINT64_MAX; // impossible offset into bitmap, so good for errors
    }

    uint64_t PhysicalPageFrameAllocator::FindFreePages(uint64_t count) {
        uint64_t next = 0;
        if (count == 0) return UINT64_MAX; // impossible offset into bitmap, so good for errors
        for (uint64_t i = 0; i < (m_Bitmap.GetSize() << 3); i+=next) {
            next = 1;
            if (m_Bitmap[i] == 0) {
                if (count == 1) {
                    if (m_nextFree == i)
                        m_nextFree = UINT64_MAX;
                    return i * 8;
                }
                bool found = true;
                next = 1;
                while (next <= count) {
                    if (m_Bitmap[i + next] == 1) {
                        found = false;
                        break;
                    }
                    if (count <= next) break; // check if next > count on next iteration
                    next++;
                }
                if (found) {
                    if (m_nextFree >= i && (i + count) > m_nextFree)
                        m_nextFree = UINT64_MAX; // reset it
                    return i; // return start page index of the block
                }
            }
        }
        return UINT64_MAX; // impossible offset into bitmap, so good for errors
    }

}