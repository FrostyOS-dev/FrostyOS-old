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

#include "PageManager.hpp"
#include "Memory/PageObject.hpp"
#include "Memory/kmalloc.hpp"
#include "newdelete.hpp"
#include "PhysicalPageFrameAllocator.hpp"
#include "VirtualPageManager.hpp"
#include "spinlock.h"

#include <HAL/hal.hpp>

#include <util.h>

PageManager* g_KPM = nullptr;

PageManager::PageManager() : m_allocated_objects(true), m_allocated_object_count(0), m_Vregion(), m_VPM(), m_PT(false, this), m_mode(false), m_auto_expand(false), m_page_object_pool_used(false), m_lock(0) {
    
}

PageManager::PageManager(const VirtualRegion& region, VirtualPageManager* VPM, bool mode, bool auto_expand) : m_allocated_objects(true), m_allocated_object_count(0), m_Vregion(region), m_VPM(VPM), m_PT(mode, this), m_mode(mode), m_auto_expand(mode && auto_expand), m_page_object_pool_used(false), m_lock(0) {
    if (!PageObjectPool_HasBeenInitialised())
        PageObjectPool_Init();
}

PageManager::~PageManager() {
    // if page object pool was used then we panic
    if (m_page_object_pool_used) {
        if (m_mode) {
            PANIC("USER PageManager illegal destruction. PageManager cannot be destroyed if page object pool has been used.");
        }
        else {
            PANIC("SUPERVISOR PageManager illegal destruction. PageManager cannot be destroyed if page object pool has been used.");
        }
    }
    spinlock_acquire(&m_lock);
    m_allocated_objects.clear();
    spinlock_release(&m_lock);
}

void PageManager::InitPageManager(const VirtualRegion& region, VirtualPageManager* VPM, bool mode, bool auto_expand) {
    spinlock_acquire(&m_lock);
    m_allocated_objects.clear();
    m_allocated_object_count = 0;
    m_Vregion = region;
    m_VPM = VPM;
    m_PT = PageTable(mode, this);
    m_mode = mode;
    m_page_object_pool_used = false;
    m_auto_expand = mode && auto_expand;
    spinlock_release(&m_lock);
    if (!PageObjectPool_HasBeenInitialised())
        PageObjectPool_Init();
}

void* PageManager::AllocatePage(PagePermissions perms, void* addr) {
    spinlock_acquire(&m_lock);
    if (addr != nullptr) {
        PageObject* obj = FindObject(VirtualRegion(addr, PAGE_SIZE));
        if (obj != nullptr) {
            if ((obj->flags & PO_INUSE) == 0 && (obj->flags & PO_STANDBY) != 0) {
                // split the object
                if (obj->virtual_address < addr) {
                    // split the object from the left
                    uint64_t new_page_count = (uint64_t)addr - (uint64_t)obj->virtual_address;
                    PageObject* new_obj = CreateObject({obj->virtual_address, new_page_count, obj->flags, obj->perms});
                    obj->virtual_address = addr;
                    obj->page_count -= new_page_count;
                    RemoveObject(obj);
                    InsertObject(new_obj);
                    InsertObject(obj); // reinsert the object because the address has changed
                }
                if (((uint64_t)obj->virtual_address + obj->page_count * PAGE_SIZE) > ((uint64_t)addr + PAGE_SIZE)) {
                    // split the object from the right
                    uint64_t new_page_count = (uint64_t)obj->virtual_address + obj->page_count * PAGE_SIZE - (uint64_t)addr;
                    PageObject* new_obj = CreateObject({addr, new_page_count, obj->flags, obj->perms});
                    obj->page_count -= new_page_count;
                    InsertObject(new_obj);
                    obj = new_obj;
                }
                obj->flags |= PO_INUSE;
                obj->flags &= ~PO_STANDBY;
                obj->perms = perms;

                // map the page
                m_PT.MapPage(g_PPFA->AllocatePage(), addr, perms);
                spinlock_release(&m_lock);
                return addr;
            }
            else {
                spinlock_release(&m_lock);
                return nullptr; // already allocated
            }
        }
    }
    // at this point we know that addr is not allocated
    void* virt_addr;
    if (addr == nullptr)
        virt_addr = m_VPM->AllocatePage();
    else
        virt_addr = m_VPM->AllocatePage(addr);
    if (virt_addr == nullptr) {
        if (m_auto_expand) {
            if (ExpandVRegionToRight(PAGE_SIZE + m_Vregion.GetSize())) {
                if (addr == nullptr)
                    virt_addr = m_VPM->AllocatePage();
                else
                    virt_addr = m_VPM->AllocatePage(addr);
            }
            if (virt_addr == nullptr) {
                spinlock_release(&m_lock);
                return nullptr;
            }
        }
        else {
            spinlock_release(&m_lock);
            return nullptr;
        }
    }
    PageObject* po = CreateObject();
    po->flags = PO_ALLOCATED | PO_INUSE;
    if (m_mode)
        po->flags |= PO_USER;
    po->virtual_address = virt_addr;
    po->page_count = 1;
    po->perms = perms;
    InsertObject(po);
    m_PT.MapPage(g_PPFA->AllocatePage(), virt_addr, perms);
    spinlock_release(&m_lock);
    return virt_addr;
}

void* PageManager::AllocatePages(uint64_t count, PagePermissions perms, void* addr) {
    spinlock_acquire(&m_lock);
    if (addr != nullptr) {
        PageObject* obj = FindObject(VirtualRegion(addr, count * PAGE_SIZE));
        if (obj != nullptr) {
            if ((obj->flags & PO_INUSE) == 0 && (obj->flags & PO_STANDBY) != 0) {
                // split the object
                if (obj->virtual_address < addr) {
                    // split the object from the left
                    uint64_t new_page_count = (uint64_t)addr - (uint64_t)obj->virtual_address;
                    PageObject* new_obj = CreateObject({obj->virtual_address, new_page_count, obj->flags, obj->perms});
                    obj->virtual_address = addr;
                    obj->page_count -= new_page_count;
                    RemoveObject(obj);
                    InsertObject(new_obj);
                    InsertObject(obj); // reinsert the object because the address has changed
                }
                if (((uint64_t)obj->virtual_address + obj->page_count * PAGE_SIZE) > ((uint64_t)addr + count * PAGE_SIZE)) {
                    // split the object from the right
                    uint64_t new_page_count = (uint64_t)obj->virtual_address + obj->page_count * PAGE_SIZE - (uint64_t)addr;
                    PageObject* new_obj = CreateObject({addr, new_page_count, obj->flags, obj->perms});
                    obj->page_count -= new_page_count;
                    InsertObject(new_obj);
                    obj = new_obj;
                }
                obj->flags |= PO_INUSE;
                obj->flags &= ~PO_STANDBY;
                obj->perms = perms;

                // map the pages
                for (uint64_t i = 0; i < count; i++)
                    m_PT.MapPage(g_PPFA->AllocatePage(), (void*)((uint64_t)addr + i * PAGE_SIZE), perms, false);
                m_PT.Flush(addr, count * PAGE_SIZE, true);
                spinlock_release(&m_lock);
                return addr;
            }
            else {
                spinlock_release(&m_lock);
                return nullptr; // already allocated
            }
        }
    }
    // at this point we know that addr is not allocated
    void* virt_addr;
    if (addr == nullptr)
        virt_addr = m_VPM->AllocatePages(count);
    else
        virt_addr = m_VPM->AllocatePages(addr, count);
    if (virt_addr == nullptr) {
        if (m_auto_expand) {
            if (ExpandVRegionToRight(count * PAGE_SIZE + m_Vregion.GetSize())) {
                if (addr == nullptr)
                    virt_addr = m_VPM->AllocatePages(count);
                else
                    virt_addr = m_VPM->AllocatePages(addr, count);
            }
            if (virt_addr == nullptr) {
                spinlock_release(&m_lock);
                return nullptr;
            }
        }
        else {
            spinlock_release(&m_lock);
            return nullptr;
        }
    }
    PageObject* po = CreateObject();
    po->flags = PO_ALLOCATED | PO_INUSE;
    if (m_mode)
        po->flags |= PO_USER;
    po->virtual_address = virt_addr;
    po->page_count = count;
    po->perms = perms;
    InsertObject(po);
    for (uint64_t i = 0; i < count; i++)
        m_PT.MapPage(g_PPFA->AllocatePage(), (void*)((uint64_t)virt_addr + i * PAGE_SIZE), perms, false);
    m_PT.Flush(virt_addr, count * PAGE_SIZE, true);
    spinlock_release(&m_lock);
    return virt_addr;
}

void* PageManager::ReservePage(PagePermissions perms, void* addr) {
    spinlock_acquire(&m_lock);
    if (addr != nullptr) {
        PageObject* obj = FindObject(VirtualRegion(addr, PAGE_SIZE));
        if (obj != nullptr) {
            spinlock_release(&m_lock);
            return nullptr;
        }
    }
    void* virt_addr;
    if (addr == nullptr)
        virt_addr = m_VPM->AllocatePage();
    else
        virt_addr = m_VPM->AllocatePage(addr);
    if (virt_addr == nullptr) {
        if (m_auto_expand) {
            if (ExpandVRegionToRight(PAGE_SIZE + m_Vregion.GetSize())) {
                if (addr == nullptr)
                    virt_addr = m_VPM->AllocatePage();
                else
                    virt_addr = m_VPM->AllocatePage(addr);
            }
            if (virt_addr == nullptr) {
                spinlock_release(&m_lock);
                return nullptr;
            }
        }
        else {
            spinlock_release(&m_lock);
            return nullptr;
        }
    }
    PageObject* po = CreateObject();
    po->flags = PO_ALLOCATED | PO_STANDBY;
    if (m_mode)
        po->flags |= PO_USER;
    po->virtual_address = virt_addr;
    po->page_count = 1;
    po->perms = perms;
    InsertObject(po);
    spinlock_release(&m_lock);
    return virt_addr;
}

void* PageManager::ReservePages(uint64_t count, PagePermissions perms, void* addr) {
    spinlock_acquire(&m_lock);
    if (addr != nullptr) {
        PageObject* obj = FindObject(VirtualRegion(addr, count * PAGE_SIZE));
        if (obj != nullptr) {
            spinlock_release(&m_lock);
            return nullptr;
        }
    }
    void* virt_addr;
    if (addr == nullptr)
        virt_addr = m_VPM->AllocatePages(count);
    else
        virt_addr = m_VPM->AllocatePages(addr, count);
    if (virt_addr == nullptr) {
        if (m_auto_expand) {
            if (ExpandVRegionToRight(count * PAGE_SIZE + m_Vregion.GetSize())) {
                if (addr == nullptr)
                    virt_addr = m_VPM->AllocatePages(count);
                else
                    virt_addr = m_VPM->AllocatePages(addr, count);
            }
            if (virt_addr == nullptr) {
                spinlock_release(&m_lock);
                return nullptr;
            }
        }
        else {
            spinlock_release(&m_lock);
            return nullptr;
        }
    }
    PageObject* po = CreateObject();
    po->flags = PO_ALLOCATED | PO_STANDBY;
    if (m_mode)
        po->flags |= PO_USER;
    po->virtual_address = virt_addr;
    po->page_count = count;
    po->perms = perms;
    InsertObject(po);
    spinlock_release(&m_lock);
    return virt_addr;
}

void PageManager::FreePage(void* addr) {
    FreePages(addr);
}

void PageManager::FreePages(void* addr) {
    spinlock_acquire(&m_lock);
    PageObject* obj = FindObject(addr);
    if (obj == nullptr) {
        spinlock_release(&m_lock);
        return;
    }
    RemoveObject(obj);
    m_VPM->UnallocatePages(addr, obj->page_count);
    for (uint64_t i = 0; i < obj->page_count; i++) {
        void* phys = m_PT.GetPhysicalAddress((void*)((uint64_t)addr + i * PAGE_SIZE));
        m_PT.UnmapPage((void*)((uint64_t)addr + i * PAGE_SIZE), false);
        g_PPFA->FreePage(phys);
    }
    m_PT.Flush(addr, obj->page_count * PAGE_SIZE, true);
    DeleteObject(obj);
    spinlock_release(&m_lock);
}

void PageManager::Remap(void* addr, PagePermissions perms) {
    spinlock_acquire(&m_lock);
    PageObject* obj = FindObject(addr);
    if (obj == nullptr) {
        spinlock_release(&m_lock);
        return;
    }
    obj->perms = perms;
    for (uint64_t i = 0; i < obj->page_count; i++)
        m_PT.RemapPage((void*)((uint64_t)addr + i * PAGE_SIZE), perms, false);
    m_PT.Flush(addr, obj->page_count * PAGE_SIZE, true);
    spinlock_release(&m_lock);
}

void* PageManager::MapPage(void* phys, PagePermissions perms, void* addr) {
    spinlock_acquire(&m_lock);
    if (addr != nullptr) {
        PageObject* obj = FindObject(VirtualRegion(addr, PAGE_SIZE));
        if (obj != nullptr) {
            if ((obj->flags & PO_INUSE) == 0 && (obj->flags & PO_STANDBY) != 0) {
                // split the object
                if (obj->virtual_address < addr) {
                    // split the object from the left
                    uint64_t new_page_count = (uint64_t)addr - (uint64_t)obj->virtual_address;
                    PageObject* new_obj = CreateObject({obj->virtual_address, new_page_count, obj->flags, obj->perms});
                    obj->virtual_address = addr;
                    obj->page_count -= new_page_count;
                    RemoveObject(obj);
                    InsertObject(new_obj);
                    InsertObject(obj); // reinsert the object because the address has changed
                }
                if (((uint64_t)obj->virtual_address + obj->page_count * PAGE_SIZE) > ((uint64_t)addr + PAGE_SIZE)) {
                    // split the object from the right
                    uint64_t new_page_count = (uint64_t)obj->virtual_address + obj->page_count * PAGE_SIZE - (uint64_t)addr;
                    PageObject* new_obj = CreateObject({addr, new_page_count, obj->flags, obj->perms});
                    obj->page_count -= new_page_count;
                    InsertObject(new_obj);
                    obj = new_obj;
                }
                obj->flags |= PO_INUSE;
                obj->flags &= ~PO_STANDBY;
                obj->perms = perms;

                // map the page
                m_PT.MapPage(phys, addr, perms);
                spinlock_release(&m_lock);
                return addr;
            }
            else {
                spinlock_release(&m_lock);
                return nullptr; // already allocated
            }
        }
    }
    // at this point we know that addr is not allocated
    void* virt_addr;
    if (addr == nullptr)
        virt_addr = m_VPM->AllocatePage();
    else
        virt_addr = m_VPM->AllocatePage(addr);
    if (virt_addr == nullptr) {
        if (m_auto_expand) {
            if (ExpandVRegionToRight(PAGE_SIZE + m_Vregion.GetSize())) {
                if (addr == nullptr)
                    virt_addr = m_VPM->AllocatePage();
                else
                    virt_addr = m_VPM->AllocatePage(addr);
            }
            if (virt_addr == nullptr) {
                spinlock_release(&m_lock);
                return nullptr;
            }
        }
        else {
            spinlock_release(&m_lock);
            return nullptr;
        }
    }
    PageObject* po = CreateObject();
    po->flags = PO_ALLOCATED | PO_INUSE;
    if (m_mode)
        po->flags |= PO_USER;
    po->virtual_address = virt_addr;
    po->page_count = 1;
    po->perms = perms;
    InsertObject(po);
    m_PT.MapPage(phys, virt_addr, perms);
    spinlock_release(&m_lock);
    return virt_addr;
}

void* PageManager::MapPages(void* phys, uint64_t count, PagePermissions perms, void* addr) {
    spinlock_acquire(&m_lock);
    if (addr != nullptr) {
        PageObject* obj = FindObject(VirtualRegion(addr, count * PAGE_SIZE));
        if (obj != nullptr) {
            if ((obj->flags & PO_INUSE) == 0 && (obj->flags & PO_STANDBY) != 0) {
                // split the object
                if (obj->virtual_address < addr) {
                    // split the object from the left
                    RemoveObject(obj);
                    uint64_t new_page_count = (uint64_t)addr - (uint64_t)obj->virtual_address;
                    PageObject* new_obj = CreateObject({obj->virtual_address, new_page_count, obj->flags, obj->perms});
                    obj->virtual_address = addr;
                    obj->page_count -= new_page_count;
                    InsertObject(new_obj);
                    InsertObject(obj); // reinsert the object because the address has changed
                }
                if (((uint64_t)obj->virtual_address + obj->page_count * PAGE_SIZE) > ((uint64_t)addr + count * PAGE_SIZE)) {
                    // split the object from the right
                    uint64_t new_page_count = (uint64_t)obj->virtual_address + obj->page_count * PAGE_SIZE - (uint64_t)addr;
                    PageObject* new_obj = CreateObject({addr, new_page_count, obj->flags, obj->perms});
                    obj->page_count -= new_page_count;
                    InsertObject(new_obj);
                    obj = new_obj;
                }
                obj->flags |= PO_INUSE;
                obj->flags &= ~PO_STANDBY;
                obj->perms = perms;

                // map the pages
                for (uint64_t i = 0; i < count; i++)
                    m_PT.MapPage((void*)((uint64_t)phys + i * count), (void*)((uint64_t)addr + i * PAGE_SIZE), perms, false);
                m_PT.Flush(addr, count * PAGE_SIZE, true);
                spinlock_release(&m_lock);
                return addr;
            }
            else {
                spinlock_release(&m_lock);
                return nullptr; // already allocated
            }
        }
    }
    // at this point we know that addr is not allocated
    void* virt_addr;
    if (addr == nullptr)
        virt_addr = m_VPM->AllocatePages(count);
    else
        virt_addr = m_VPM->AllocatePages(addr, count);
    if (virt_addr == nullptr) {
        if (m_auto_expand) {
            if (ExpandVRegionToRight(count * PAGE_SIZE + m_Vregion.GetSize())) {
                if (addr == nullptr)
                    virt_addr = m_VPM->AllocatePages(count);
                else
                    virt_addr = m_VPM->AllocatePages(addr, count);
            }
            if (virt_addr == nullptr) {
                spinlock_release(&m_lock);
                return nullptr;
            }
        }
        else {
            spinlock_release(&m_lock);
            return nullptr;
        }
    }
    PageObject* po = CreateObject();
    po->flags = PO_ALLOCATED | PO_INUSE;
    if (m_mode)
        po->flags |= PO_USER;
    po->virtual_address = virt_addr;
    po->page_count = count;
    po->perms = perms;
    InsertObject(po);
    for (uint64_t i = 0; i < count; i++)
        m_PT.MapPage((void*)((uint64_t)phys + i * PAGE_SIZE), (void*)((uint64_t)virt_addr + i * PAGE_SIZE), perms, false);
    m_PT.Flush(virt_addr, count * PAGE_SIZE, true);
    spinlock_release(&m_lock);
    return virt_addr;
}

void PageManager::UnmapPage(void* addr) {
    UnmapPages(addr);
}

void PageManager::UnmapPages(void* addr) {
    spinlock_acquire(&m_lock);
    PageObject* obj = FindObject(addr);
    if (obj == nullptr) {
        spinlock_release(&m_lock);
        return;
    }
    RemoveObject(obj);
    m_VPM->UnallocatePages(addr, obj->page_count);
    for (uint64_t i = 0; i < obj->page_count; i++)
        m_PT.UnmapPage((void*)((uint64_t)addr + i * PAGE_SIZE), false);
    m_PT.Flush(addr, obj->page_count * PAGE_SIZE, true);
    DeleteObject(obj);
    spinlock_release(&m_lock);
}

bool PageManager::ExpandVRegionToRight(size_t new_size) {
    spinlock_acquire(&m_lock);
    if (new_size <= m_Vregion.GetSize()) {
        spinlock_release(&m_lock);
        return false; // bad size
    }
    if (!m_VPM->AttemptToExpandRight(new_size)) {
        spinlock_release(&m_lock);
        return false; // failed to expand
    }
    m_Vregion.ExpandRight(new_size);
    spinlock_release(&m_lock);
    return true;
}

const VirtualRegion& PageManager::GetRegion() const {
    return m_Vregion;
}

const PageTable& PageManager::GetPageTable() const {
    return m_PT;
}
