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

#include "PhysicalPageFrameAllocator.hpp"

#include <util.h>
#include <stdio.h>


#include <HAL/hal.hpp>

#include "PagingUtil.hpp"
#include "arch/x86_64/panic.hpp"

PhysicalPageFrameAllocator* g_PPFA = nullptr;
uint8_t g_EarlyBitmap[128 * 1024] = {0};

/* PhysicalPageFrameAllocator class */

/* Public Methods */

PhysicalPageFrameAllocator::PhysicalPageFrameAllocator() : m_Bitmap(), m_FreeMem(0), m_ReservedMem(0), m_UsedMem(0), m_MemSize(0), m_nextFree(UINT64_MAX), m_fullyInitialised(false), m_BitmapLock(0), m_globalLock(0) {

}

PhysicalPageFrameAllocator::~PhysicalPageFrameAllocator() {
    
}

void PhysicalPageFrameAllocator::EarlyInit(const MemoryMapEntry* FirstMemoryMapEntry, const size_t MemoryMapEntryCount, uint64_t MemorySize) {
    // we don't bother with locks in the early init, as this is way before we have any multithreading

    m_fullyInitialised = false;
    
    // Setup
    m_MemSize = (size_t)MemorySize;
    if (m_MemSize > GiB(4))
        m_MemSize = GiB(4);
    m_FreeMem = 0;
    m_ReservedMem = 0;
    m_UsedMem = 0;
    m_nextFree = UINT64_MAX;

    size_t BitmapSize = DIV_ROUNDUP(m_MemSize, (PAGE_SIZE * 8));
    m_Bitmap.SetSize(BitmapSize);
    m_Bitmap.SetBuffer(g_EarlyBitmap);

    memset(m_Bitmap.GetBuffer(), 0xFF, BitmapSize); // we set all bits to 1, as that means the page is reserved

    // Fill bitmap
    for (uint64_t i = 0; i < MemoryMapEntryCount; i++) {
        MemoryMapEntry* entry = (MemoryMapEntry*)((uint64_t)FirstMemoryMapEntry + (i * MEMORY_MAP_ENTRY_SIZE));
        // We don't need to check if the address is in bitmap range because the bitmap has protections
        if (entry->type == FROSTYOS_MEMORY_FREE) {
            m_FreeMem += entry->length;
            if (entry->length > PAGE_SIZE) {
                for (uint64_t j = 0; j < entry->length; j += PAGE_SIZE)
                    m_Bitmap.Set((entry->Address + j) / PAGE_SIZE, false);
            }
            else
                m_Bitmap.Set(entry->Address / PAGE_SIZE, false);
        }
        else {
            m_ReservedMem += entry->length;
            uint64_t length = ALIGN_UP(entry->length, PAGE_SIZE);
            if (length > PAGE_SIZE) {
                for (uint64_t j = 0; j < length; j += PAGE_SIZE)
                    m_Bitmap.Set((j + entry->Address) / PAGE_SIZE, true);
            }
            else
                m_Bitmap.Set(entry->Address / PAGE_SIZE, true);
        }
    }

    if (m_Bitmap[0] == 0) {
        m_ReservedMem += PAGE_SIZE;
        m_Bitmap.Set(0, true);
    }
}

void PhysicalPageFrameAllocator::FullInit(const MemoryMapEntry* FirstMemoryMapEntry, const size_t MemoryMapEntryCount, uint64_t MemorySize) {
    // We still don't need to worry about locks here, as this is still before we have any multithreading
    
    // Setup
    m_MemSize = MemorySize;

    if (m_MemSize <= GiB(4)) {
        m_fullyInitialised = true;
        spinlock_init(&m_BitmapLock);
        spinlock_init(&m_globalLock);
        return;
    }

    size_t BitmapSize = DIV_ROUNDUP(m_MemSize, (PAGE_SIZE * 8));
    void* BitmapAddress = AllocatePages(DIV_ROUNDUP(BitmapSize, PAGE_SIZE));
    assert(BitmapAddress != nullptr);
    m_Bitmap.SetSize(BitmapSize);
    m_Bitmap.SetBuffer((uint8_t*)to_HHDM(BitmapAddress));
    memset(m_Bitmap.GetBuffer(), 0xFF, BitmapSize); // we set all bits to 1, as that means the page is reserved

    // Copy old bitmap
    memcpy(m_Bitmap.GetBuffer(), g_EarlyBitmap, KiB(128));

    // Fill bitmap
    for (uint64_t i = 0; i < MemoryMapEntryCount; i++) {
        MemoryMapEntry* entry = (MemoryMapEntry*)((uint64_t)FirstMemoryMapEntry + (i * MEMORY_MAP_ENTRY_SIZE));
        if ((entry->Address + entry->length) <= GiB(4))
            continue; // ignore entries below 4GiB
        if (entry->type == FROSTYOS_MEMORY_FREE) {
            m_FreeMem += entry->length;
            if (entry->length > PAGE_SIZE) {
                for (uint64_t j = 0; j < entry->length; j += PAGE_SIZE) {
                    if ((entry->Address + j) < GiB(4))
                        continue; // skip entries below 4GiB
                    m_Bitmap.Set((j + entry->Address) / PAGE_SIZE, false);
                }
            }
            else {
                if (!(entry->Address < GiB(4)))
                    m_Bitmap.Set(entry->Address / PAGE_SIZE, false);
            }
        }
        else {
            // Address and length must be made page aligned because they might not be
            m_ReservedMem += entry->length;
            uint64_t length = ALIGN_UP(entry->length, PAGE_SIZE);
            uint64_t addr = ALIGN_DOWN(entry->Address, PAGE_SIZE);
            if (length > 4096) {
                for (uint64_t j = 0; j < length; j += 4096) {
                    if ((addr + j) < GiB(4))
                        continue; // skip entries below 4GiB
                    m_Bitmap.Set((j + addr) / PAGE_SIZE, true);
                }
            }
            else {
                if (!(addr < GiB(4)))
                    m_Bitmap.Set(addr / PAGE_SIZE, true);
            }
        }
    }

    if (m_Bitmap[0] == 0) {
        m_ReservedMem += 0x1000;
        m_Bitmap.Set(0, true);
    }

    m_nextFree = UINT64_MAX;

    m_fullyInitialised = true;

    spinlock_init(&m_BitmapLock);
    spinlock_init(&m_globalLock);
}

void* PhysicalPageFrameAllocator::AllocatePage() {
    uint64_t index = FindFreePage();
    if (index == UINT64_MAX) { // Out of memory. Panic immediately
        PANIC("OUT OF MEMORY. No physical pages are available.");
    }
    if (index == 0) { // Attempted to allocate the first page of memory. This is not allowed. Panic immediately
        PANIC("Attempted to allocate the first page of memory. This is not allowed.");
    }
    LockPage((void*)(index * 4096));
    return (void*)(index * 4096);
}

void* PhysicalPageFrameAllocator::AllocatePages(uint64_t count) {
    uint64_t index = FindFreePages(count);
    if (index == UINT64_MAX)
        return nullptr;
    if (index == 0) { // Attempted to allocate the first page of memory. This is not allowed. Panic immediately
        PANIC("Attempted to allocate the first page of memory. This is not allowed.");
    }
    LockPages((void*)(index * 4096), count);
    return (void*)(index * 4096);
}

void PhysicalPageFrameAllocator::ReservePage(void* page) {
    if (m_fullyInitialised)
        spinlock_acquire(&m_BitmapLock);
    if (m_Bitmap[((uint64_t)page >> 12)]) {
        if (m_fullyInitialised)
            spinlock_release(&m_BitmapLock);
        return;
    }
    m_Bitmap.Set(((uint64_t)page >> 12), true);
    if (m_fullyInitialised) {
        spinlock_release(&m_BitmapLock);
        spinlock_acquire(&m_globalLock);
    }
    m_FreeMem -= 4096;
    m_ReservedMem += 4096;
    if (m_nextFree == ((uint64_t)page >> 12))
        m_nextFree = UINT64_MAX;
    if (m_fullyInitialised)
        spinlock_release(&m_globalLock);
}

void PhysicalPageFrameAllocator::ReservePages(void* start, uint64_t count) {
    for (uint64_t i = 0; i < count; i++) {
        ReservePage((void*)(i * 4096 + (uint64_t)start));
    }
}

void PhysicalPageFrameAllocator::UnreservePage(void* page) {
    if (m_fullyInitialised)
        spinlock_acquire(&m_BitmapLock);
    if (!m_Bitmap[((uint64_t)page >> 12)]) {
        if (m_fullyInitialised)
            spinlock_release(&m_BitmapLock);
        return;
    }
    m_Bitmap.Set(((uint64_t)page >> 12), false);
    if (m_fullyInitialised) {
        spinlock_release(&m_BitmapLock);
        spinlock_acquire(&m_globalLock);
    }
    m_FreeMem += 4096;
    m_ReservedMem -= 4096;
    m_nextFree = ((uint64_t)page >> 12);
    if (m_fullyInitialised)
        spinlock_release(&m_globalLock);
}

void PhysicalPageFrameAllocator::UnreservePages(void* start, uint64_t count) {
    for (uint64_t i = 0; i < count; i++) {
        UnreservePage((void*)(i * 4096 + (uint64_t)start));
    }
}

void PhysicalPageFrameAllocator::FreePage(void* page) {
    if (m_fullyInitialised)
        spinlock_acquire(&m_BitmapLock);
    if (!m_Bitmap[((uint64_t)page >> 12)]) {
        if (m_fullyInitialised)
            spinlock_release(&m_BitmapLock);
        return;
    }
    m_Bitmap.Set(((uint64_t)page >> 12), false);
    if (m_fullyInitialised) {
        spinlock_release(&m_BitmapLock);
        spinlock_acquire(&m_globalLock);
    }
    m_FreeMem += 4096;
    m_UsedMem -= 4096;
    m_nextFree = ((uint64_t)page >> 12);
    if (m_fullyInitialised)
        spinlock_release(&m_globalLock);
}

void PhysicalPageFrameAllocator::FreePages(void* start, uint64_t count) {
    for (uint64_t i = 0; i < count; i++) {
        FreePage((void*)(i * 4096 + (uint64_t)start));
    }
}

/* Private Methods */

void PhysicalPageFrameAllocator::LockPage(void* page) {
    if (m_fullyInitialised)
        spinlock_acquire(&m_BitmapLock);
    if (m_Bitmap[((uint64_t)page >> 12)]) {
        if (m_fullyInitialised)
            spinlock_release(&m_BitmapLock);
        return;
    }
    m_Bitmap.Set(((uint64_t)page >> 12), true);
    if (m_fullyInitialised) {
        spinlock_release(&m_BitmapLock);
        spinlock_acquire(&m_globalLock);
    }
    m_FreeMem -= 4096;
    m_UsedMem += 4096;
    if (m_fullyInitialised)
        spinlock_release(&m_globalLock);
}

void PhysicalPageFrameAllocator::LockPages(void* start, uint64_t count) {
    for (uint64_t i = 0; i < count; i++) {
        LockPage((void*)(i * 4096 + (uint64_t)start));
    }
}


void PhysicalPageFrameAllocator::UnlockPage(void* page) {
    if (m_fullyInitialised)
        spinlock_acquire(&m_BitmapLock);
    if (!m_Bitmap[((uint64_t)page >> 12)]) {
        if (m_fullyInitialised)
            spinlock_release(&m_BitmapLock);
        return;
    }
    m_Bitmap.Set(((uint64_t)page >> 12), false);
    if (m_fullyInitialised) {
        spinlock_release(&m_BitmapLock);
        spinlock_acquire(&m_globalLock);
    }
    m_FreeMem += 4096;
    m_UsedMem -= 4096;
    if (m_fullyInitialised)
        spinlock_release(&m_globalLock);
}

void PhysicalPageFrameAllocator::UnlockPages(void* start, uint64_t count) {
    for (uint64_t i = 0; i < count; i++) {
        UnlockPage((void*)(i * 4096 + (uint64_t)start));
    }
}

uint64_t PhysicalPageFrameAllocator::FindFreePage() {
    if (m_fullyInitialised) {
        spinlock_acquire(&m_BitmapLock);
        spinlock_acquire(&m_globalLock);
    }
    if (m_nextFree != UINT64_MAX) {
        uint64_t i = m_nextFree;
        if ((i + 1) < (m_Bitmap.GetSize() << 3) && m_Bitmap[i + 1] == 0)
            m_nextFree = i + 1;
        else
            m_nextFree = UINT64_MAX;
        if (m_fullyInitialised) {
            spinlock_release(&m_globalLock);
            spinlock_release(&m_BitmapLock);
        }
        return i;
    }
    for (uint64_t i = 0; i < (m_Bitmap.GetSize() << 3); i++) {
        if (m_Bitmap[i] == 0) {
            if ((i + 1) < (m_Bitmap.GetSize() << 3) && m_Bitmap[i + 1] == 0)
                m_nextFree = i + 1;
            if (m_fullyInitialised) {
                spinlock_release(&m_globalLock);
                spinlock_release(&m_BitmapLock);
            }
            return i;
        }
    }
    if (m_fullyInitialised) {
        spinlock_release(&m_globalLock);
        spinlock_release(&m_BitmapLock);
    }
    return UINT64_MAX; // impossible offset into bitmap, so good for errors
}

uint64_t PhysicalPageFrameAllocator::FindFreePages(uint64_t count) {
    if (m_fullyInitialised) {
        spinlock_acquire(&m_BitmapLock);
        spinlock_acquire(&m_globalLock);
    }
    uint64_t next = 0;
    if (count == 0) return UINT64_MAX; // impossible offset into bitmap, so good for errors
    for (uint64_t i = 0; i < (m_Bitmap.GetSize() << 3); i+=next) {
        next = 1;
        if (m_Bitmap[i] == 0) {
            if (count == 1) {
                if (m_nextFree == i)
                    m_nextFree = UINT64_MAX;
                if (m_fullyInitialised) {
                    spinlock_release(&m_globalLock);
                    spinlock_release(&m_BitmapLock);
                }
                return i;
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
                if (m_fullyInitialised) {
                    spinlock_release(&m_globalLock);
                    spinlock_release(&m_BitmapLock);
                }
                return i; // return start page index of the block
            }
        }
    }
    if (m_fullyInitialised) {
        spinlock_release(&m_globalLock);
        spinlock_release(&m_BitmapLock);
    }
    return UINT64_MAX; // impossible offset into bitmap, so good for errors
}
