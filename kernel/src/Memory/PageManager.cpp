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

#include "PageManager.hpp"
#include "newdelete.hpp"
#include "PhysicalPageFrameAllocator.hpp"
#include "VirtualPageManager.hpp"
#include "PagingUtil.hpp"

#include <HAL/hal.hpp>

namespace WorldOS {

    PageManager* g_KPM = nullptr;

    PageManager::PageManager() : m_allocated_objects(nullptr), m_allocated_object_count(0), m_Vregion(), m_VPM(), m_mode(false), m_page_object_pool_used(false), m_auto_expand(false) {
        
    }

    PageManager::PageManager(const VirtualRegion& region, VirtualPageManager* VPM, bool mode, bool auto_expand) : m_allocated_objects(nullptr), m_allocated_object_count(0), m_Vregion(region), m_VPM(VPM), m_mode(mode), m_page_object_pool_used(false), m_auto_expand(mode && auto_expand) {
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
        for (uint64_t i = 0; i < m_allocated_object_count; i++) {
            PageObject* object = m_allocated_objects;
            m_allocated_objects = object->next;
            delete object;
        }
    }

    void PageManager::InitPageManager(const VirtualRegion& region, VirtualPageManager* VPM, bool mode, bool auto_expand) {
        m_allocated_objects = nullptr;
        m_allocated_object_count = 0;
        m_Vregion = region;
        m_VPM = VPM;
        m_mode = mode;
        m_page_object_pool_used = false;
        m_auto_expand = mode && auto_expand;
        if (!PageObjectPool_HasBeenInitialised())
            PageObjectPool_Init();
    }

    void* PageManager::AllocatePage(PagePermissions perms, void* addr) {
        if (addr != nullptr) {
            PageObject* object = m_allocated_objects;
            for (uint64_t i = 0; i < m_allocated_object_count; i++) {
                if (object == nullptr) // should NEVER happen
                    return nullptr;
                VirtualRegion temp_region = VirtualRegion(object->virtual_address, object->page_count * PAGE_SIZE);
                if (temp_region.IsInside(addr, PAGE_SIZE) && (object->flags & PO_STANDBY) && !(object->flags & PO_INUSE) && object->perms == perms) {
                    if (addr > object->virtual_address) {
                        PageObject* po;
                        if (NewDeleteInitialised())
                            po = new PageObject;
                        else {
                            po = PageObjectPool_Allocate();
                            m_page_object_pool_used = true;
                        }
                        if (po == nullptr)
                            return nullptr;
                        po->virtual_address = object->virtual_address;
                        po->perms = perms;
                        po->flags = object->flags;
                        po->page_count = ((uint64_t)addr - (uint64_t)(object->virtual_address)) >> 12; // FIXME: don't assume page size
                        if (!InsertObject(po)) {
                            if (PageObjectPool_IsInPool(po))
                                PageObjectPool_Free(po);
                            else if (NewDeleteInitialised())
                                delete po;
                            return nullptr;
                        }
                        object->page_count -= ((uint64_t)addr - (uint64_t)(object->virtual_address)) >> 12; // FIXME: don't assume page size
                        object->virtual_address = addr;
                    }

                    if (1 < object->page_count) {
                        PageObject* po;
                        if (NewDeleteInitialised())
                            po = new PageObject;
                        else {
                            po = PageObjectPool_Allocate();
                            m_page_object_pool_used = true;
                        }
                        if (po == nullptr)
                            return nullptr;
                        po->virtual_address = (void*)((uint64_t)(object->virtual_address) + PAGE_SIZE);
                        po->perms = perms;
                        po->flags = object->flags;
                        po->page_count = object->page_count - 1;
                        if (!InsertObject(po)) {
                            if (PageObjectPool_IsInPool(po))
                                PageObjectPool_Free(po);
                            else if (NewDeleteInitialised())
                                delete po;
                            return nullptr;
                        }
                        object->page_count = 1;
                    }

                    PageObject_UnsetFlag(object, PO_STANDBY);
                    PageObject_SetFlag(object, PO_INUSE);
                    
                    uint32_t page_perms = 1;
                    if (m_mode)
                        page_perms |= 4;
                    switch (perms) {
                    case PagePermissions::READ:
                        page_perms |= 0x8000000; // No execute
                        break; // not possible to set read flag
                    case PagePermissions::WRITE:
                    case PagePermissions::READ_WRITE:
                        page_perms |= 0x8000002; // Write, No execute
                        break;
                    case PagePermissions::EXECUTE:
                    case PagePermissions::READ_EXECUTE:
                        break;
                    default:
                        page_perms = 0;
                        break;
                    }
                    MapPage(g_PPFA->AllocatePage(), addr, page_perms);
                    return addr;
                }
                object = object->next;
            }
        }
        void* virt_addr;
        if (addr == nullptr)
            virt_addr = m_VPM->AllocatePage();
        else {
            PageObject* object = m_allocated_objects;
            for (uint64_t i = 0; i < m_allocated_object_count; i++) {
                if (object == nullptr) // should NEVER happen
                    return nullptr;
                if (object->virtual_address == addr)
                    return nullptr;
                object = object->next;
            }
            virt_addr = m_VPM->AllocatePage(addr);
        }
        if (virt_addr == nullptr) {
            if (m_auto_expand) {
                if (ExpandVRegionToRight(PAGE_SIZE + m_Vregion.GetSize())) {
                    if (addr == nullptr)
                        virt_addr = m_VPM->AllocatePage();
                    else
                        virt_addr = m_VPM->AllocatePage(addr);
                }
                if (virt_addr == nullptr)
                    return nullptr;
            }
            else
                return nullptr;
        }
        PageObject* po = m_allocated_objects;
        while (po != nullptr)
            po = po->next;
        if (NewDeleteInitialised())
            po = new PageObject;
        else {
            po = PageObjectPool_Allocate();
            m_page_object_pool_used = true;
        }
        if (po == nullptr) {
            m_VPM->UnallocatePage(virt_addr);
            return nullptr;
        }
        PageObject_SetFlag(po, PO_ALLOCATED);
        if (m_mode)
            PageObject_SetFlag(po, PO_USER);
        PageObject_SetFlag(po, PO_INUSE);
        po->virtual_address = virt_addr;
        po->page_count = 1;
        if (m_allocated_object_count > 0) {
            PageObject* previous = PageObject_GetPrevious(m_allocated_objects, po);
            if (previous == nullptr) {
                if (PageObjectPool_IsInPool(po))
                    PageObjectPool_Free(po);
                else if (NewDeleteInitialised())
                    delete po;
                m_VPM->UnallocatePage(virt_addr);
                return nullptr;
            }
            else
                previous->next = po;
        }
        else
            m_allocated_objects = po;
        m_allocated_object_count++;
        uint32_t page_perms = 1;
        if (m_mode)
            page_perms |= 4;
        switch (perms) {
        case PagePermissions::READ:
            page_perms |= 0x8000000; // No execute
            break; // not possible to set read flag
        case PagePermissions::WRITE:
        case PagePermissions::READ_WRITE:
            page_perms |= 0x8000002; // Write, No execute
            break;
        case PagePermissions::EXECUTE:
        case PagePermissions::READ_EXECUTE:
            break;
        default:
            page_perms = 0;
            break;
        }
        po->perms = perms;
        MapPage(g_PPFA->AllocatePage(), virt_addr, page_perms);
        return virt_addr;
    }

    void* PageManager::AllocatePages(uint64_t count, PagePermissions perms, void* addr) {
        if (count == 1)
            return AllocatePage(perms, addr);
        if (addr != nullptr) {
            PageObject* object = m_allocated_objects;
            for (uint64_t i = 0; i < m_allocated_object_count; i++) {
                if (object == nullptr) // should NEVER happen
                    return nullptr;
                VirtualRegion temp_region = VirtualRegion(object->virtual_address, object->page_count * PAGE_SIZE);
                if (temp_region.IsInside(addr, count * PAGE_SIZE) && (object->flags & PO_STANDBY) && !(object->flags & PO_INUSE) && object->perms == perms) {
                    if (addr > object->virtual_address) {
                        PageObject* po;
                        if (NewDeleteInitialised())
                            po = new PageObject;
                        else {
                            po = PageObjectPool_Allocate();
                            m_page_object_pool_used = true;
                        }
                        if (po == nullptr)
                            return nullptr;
                        po->virtual_address = object->virtual_address;
                        po->perms = perms;
                        po->flags = object->flags;
                        po->page_count = ((uint64_t)addr - (uint64_t)(object->virtual_address)) >> 12; // FIXME: don't assume page size
                        if (!InsertObject(po)) {
                            if (PageObjectPool_IsInPool(po))
                                PageObjectPool_Free(po);
                            else if (NewDeleteInitialised())
                                delete po;
                            return nullptr;
                        }
                        object->page_count -= ((uint64_t)addr - (uint64_t)(object->virtual_address)) >> 12; // FIXME: don't assume page size
                        object->virtual_address = addr;
                    }

                    if (count < object->page_count) {
                        PageObject* po;
                        if (NewDeleteInitialised())
                            po = new PageObject;
                        else {
                            po = PageObjectPool_Allocate();
                            m_page_object_pool_used = true;
                        }
                        if (po == nullptr)
                            return nullptr;
                        po->virtual_address = (void*)((uint64_t)(object->virtual_address) + count * PAGE_SIZE);
                        po->perms = perms;
                        po->flags = object->flags;
                        po->page_count = object->page_count - count;
                        if (!InsertObject(po)) {
                            if (PageObjectPool_IsInPool(po))
                                PageObjectPool_Free(po);
                            else if (NewDeleteInitialised())
                                delete po;
                            return nullptr;
                        }
                        object->page_count = count;
                    }

                    PageObject_UnsetFlag(object, PO_STANDBY);
                    PageObject_SetFlag(object, PO_INUSE);
                    
                    uint32_t page_perms = 1;
                    if (m_mode)
                        page_perms |= 4;
                    switch (perms) {
                    case PagePermissions::READ:
                        page_perms |= 0x8000000; // No execute
                        break; // not possible to set read flag
                    case PagePermissions::WRITE:
                    case PagePermissions::READ_WRITE:
                        page_perms |= 0x8000002; // Write, No execute
                        break;
                    case PagePermissions::EXECUTE:
                    case PagePermissions::READ_EXECUTE:
                        break;
                    default:
                        page_perms = 0;
                        break;
                    }
                    for (uint64_t j = 0; j < count; j++)
                        MapPage(g_PPFA->AllocatePage(), (void*)((uint64_t)addr + j * 0x1000), page_perms);
                    return addr;
                }
                object = object->next;
            }
        }
        void* virt_addr;
        if (addr == nullptr)
            virt_addr = m_VPM->AllocatePages(count);
        else {
            if (!m_Vregion.IsInside(addr, count * PAGE_SIZE))
                return nullptr;
            PageObject* object = m_allocated_objects;
            for (uint64_t i = 0; i < m_allocated_object_count; i++) {
                if (object == nullptr) // should NEVER happen
                    return nullptr;
                VirtualRegion temp_region = VirtualRegion(object->virtual_address, object->page_count * PAGE_SIZE);
                if (temp_region.IsInside(addr, count * PAGE_SIZE))
                    return nullptr;
                object = object->next;
            }
            virt_addr = m_VPM->AllocatePages(addr, count);
        }
        if (virt_addr == nullptr) {
            if (m_auto_expand) {
                if (ExpandVRegionToRight((PAGE_SIZE * count) + m_Vregion.GetSize())) {
                    if (addr == nullptr)
                        virt_addr = m_VPM->AllocatePages(count);
                    else
                        virt_addr = m_VPM->AllocatePages(addr, count);
                }
                if (virt_addr == nullptr)
                    return nullptr;
            }
            else
                return nullptr;
        }
        PageObject* po = m_allocated_objects;
        while (po != nullptr)
            po = po->next;
        if (NewDeleteInitialised())
            po = new PageObject;
        else {
            po = PageObjectPool_Allocate();
            m_page_object_pool_used = true;
        }
        if (po == nullptr) {
            m_VPM->UnallocatePages(virt_addr, count);
            return nullptr;
        }
        PageObject_SetFlag(po, PO_ALLOCATED);
        if (m_mode)
            PageObject_SetFlag(po, PO_USER);
        PageObject_SetFlag(po, PO_INUSE);
        po->virtual_address = virt_addr;
        po->page_count = count;
        if (m_allocated_object_count > 0) {
            PageObject* previous = PageObject_GetPrevious(m_allocated_objects, po);
            if (previous == nullptr) {
                if (PageObjectPool_IsInPool(po))
                    PageObjectPool_Free(po);
                else if (NewDeleteInitialised())
                    delete po;
                m_VPM->UnallocatePages(virt_addr, count);
                return nullptr;
            }
            else
                previous->next = po;
        }
        else
            m_allocated_objects = po;
        m_allocated_object_count++;
        uint32_t page_perms = 1;
        if (m_mode)
            page_perms |= 4;
        switch (perms) {
        case PagePermissions::READ:
            page_perms |= 0x8000000; // No execute
            break; // not possible to set read flag
        case PagePermissions::WRITE:
        case PagePermissions::READ_WRITE:
            page_perms |= 0x8000002; // Write, No execute
            break;
        case PagePermissions::EXECUTE:
        case PagePermissions::READ_EXECUTE:
            break;
        default:
            page_perms = 0;
            break;
        }
        po->perms = perms;
        for (uint64_t i = 0; i < count; i++)
            MapPage(g_PPFA->AllocatePage(), (void*)((uint64_t)virt_addr + i * 0x1000), page_perms);
        return virt_addr;
    }

    void* PageManager::ReservePage(PagePermissions perms, void* addr) {
        void* virt_addr;
        if (addr == nullptr)
            virt_addr = m_VPM->AllocatePage();
        else {
            PageObject* object = m_allocated_objects;
            for (uint64_t i = 0; i < m_allocated_object_count; i++) {
                if (object == nullptr) // should NEVER happen
                    return nullptr;
                if (object->virtual_address == addr)
                    return nullptr;
                object = object->next;
            }
            virt_addr = m_VPM->AllocatePage(addr);
        }
        if (virt_addr == nullptr) {
            if (m_auto_expand) {
                if (ExpandVRegionToRight(PAGE_SIZE + m_Vregion.GetSize())) {
                    if (addr == nullptr)
                        virt_addr = m_VPM->AllocatePage();
                    else
                        virt_addr = m_VPM->AllocatePage(addr);
                }
                if (virt_addr == nullptr)
                    return nullptr;
            }
            else
                return nullptr;
        }
        PageObject* po = m_allocated_objects;
        while (po != nullptr)
            po = po->next;
        if (NewDeleteInitialised())
            po = new PageObject;
        else {
            po = PageObjectPool_Allocate();
            m_page_object_pool_used = true;
        }
        if (po == nullptr) {
            m_VPM->UnallocatePage(virt_addr);
            return nullptr;
        }
        PageObject_SetFlag(po, PO_ALLOCATED);
        if (m_mode)
            PageObject_SetFlag(po, PO_USER);
        PageObject_SetFlag(po, PO_STANDBY);
        po->virtual_address = virt_addr;
        po->page_count = 1;
        if (m_allocated_object_count > 0) {
            PageObject* previous = PageObject_GetPrevious(m_allocated_objects, po);
            if (previous == nullptr) {
                if (PageObjectPool_IsInPool(po))
                    PageObjectPool_Free(po);
                else if (NewDeleteInitialised())
                    delete po;
                m_VPM->UnallocatePage(virt_addr);
                return nullptr;
            }
            else
                previous->next = po;
        }
        else
            m_allocated_objects = po;
        m_allocated_object_count++;
        po->perms = perms;
        return virt_addr;
    }

    void* PageManager::ReservePages(uint64_t count, PagePermissions perms, void* addr) {
        if (count == 1)
            return ReservePage(perms, addr);
        void* virt_addr;
        if (addr == nullptr)
            virt_addr = m_VPM->AllocatePages(count);
        else {
            if (!m_Vregion.IsInside(addr, count * PAGE_SIZE))
                return nullptr;
            PageObject* object = m_allocated_objects;
            for (uint64_t i = 0; i < m_allocated_object_count; i++) {
                if (object == nullptr) // should NEVER happen
                    return nullptr;
                VirtualRegion temp_region = VirtualRegion(object->virtual_address, object->page_count * PAGE_SIZE);
                if (temp_region.IsInside(addr, count * PAGE_SIZE))
                    return nullptr;
                object = object->next;
            }
            virt_addr = m_VPM->AllocatePages(addr, count);
        }
        if (virt_addr == nullptr) {
            if (m_auto_expand) {
                if (ExpandVRegionToRight((PAGE_SIZE * count) + m_Vregion.GetSize())) {
                    if (addr == nullptr)
                        virt_addr = m_VPM->AllocatePages(count);
                    else
                        virt_addr = m_VPM->AllocatePages(addr, count);
                }
                if (virt_addr == nullptr)
                    return nullptr;
            }
            else
                return nullptr;
        }
        PageObject* po = m_allocated_objects;
        while (po != nullptr)
            po = po->next;
        if (NewDeleteInitialised())
            po = new PageObject;
        else {
            po = PageObjectPool_Allocate();
            m_page_object_pool_used = true;
        }
        if (po == nullptr) {
            m_VPM->UnallocatePages(virt_addr, count);
            return nullptr;
        }
        PageObject_SetFlag(po, PO_ALLOCATED);
        if (m_mode)
            PageObject_SetFlag(po, PO_USER);
        PageObject_SetFlag(po, PO_STANDBY);
        po->virtual_address = virt_addr;
        po->page_count = count;
        if (m_allocated_object_count > 0) {
            PageObject* previous = PageObject_GetPrevious(m_allocated_objects, po);
            if (previous == nullptr) {
                if (PageObjectPool_IsInPool(po))
                    PageObjectPool_Free(po);
                else if (NewDeleteInitialised())
                    delete po;
                m_VPM->UnallocatePages(virt_addr, count);
                return nullptr;
            }
            else
                previous->next = po;
        }
        else
            m_allocated_objects = po;
        m_allocated_object_count++;
        po->perms = perms;
        return virt_addr;
    }

    void PageManager::FreePage(void* addr) {
        PageObject* po = m_allocated_objects;
        while (po != nullptr) {
            if (po->virtual_address == addr && po->page_count == 1) {
                g_PPFA->FreePage(get_physaddr(addr));
                m_VPM->UnallocatePage(addr);
                UnmapPage(addr);
                PageObject* previous = PageObject_GetPrevious(m_allocated_objects, po);
                if (previous != nullptr)
                    previous->next = po->next;
                else
                    m_allocated_objects = po->next;
                if (PageObjectPool_IsInPool(po))
                    PageObjectPool_Free(po);
                else if (NewDeleteInitialised())
                    delete po;
                m_allocated_object_count--;
                return;
            }
            po = po->next;
        }
    }

    void PageManager::FreePages(void* addr) {
        PageObject* po = m_allocated_objects;
        while (po != nullptr) {
            if (po->virtual_address == addr && po->page_count > 1) {
                m_VPM->UnallocatePages(addr, po->page_count);
                for (uint64_t i = 0; i < po->page_count; i++) {
                    g_PPFA->FreePage(get_physaddr((void*)((uint64_t)addr + i * 0x1000)));
                    UnmapPage((void*)((uint64_t)addr + i * 0x1000));
                }
                PageObject* previous = PageObject_GetPrevious(m_allocated_objects, po);
                if (previous != nullptr)
                    previous->next = po->next;
                else
                    m_allocated_objects = po->next;
                if (PageObjectPool_IsInPool(po))
                    PageObjectPool_Free(po);
                else if (NewDeleteInitialised())
                    delete po;
                m_allocated_object_count--;
                return;
            }
            po = po->next;
        }
    }

    void PageManager::Remap(void* addr, PagePermissions perms) {
        PageObject* po = m_allocated_objects;
        while (po != nullptr) {
            if (po->virtual_address == addr) {
                uint32_t page_perms = 1;
                if (m_mode)
                    page_perms |= 4;
                switch (perms) {
                case PagePermissions::READ:
                    page_perms |= 0x8000000; // No execute
                    break; // not possible to set read flag
                case PagePermissions::WRITE:
                case PagePermissions::READ_WRITE:
                    page_perms |= 0x8000002; // Write, No execute
                    break;
                case PagePermissions::EXECUTE:
                case PagePermissions::READ_EXECUTE:
                    break;
                default:
                    page_perms = 0;
                    break;
                }
                po->perms = perms;
                for (uint64_t i = 0; i < po->page_count; i++)
                    RemapPage((void*)((uint64_t)addr + i * 0x1000), page_perms);
                return;
            }
            po = po->next;
        }
    }

    bool PageManager::ExpandVRegionToRight(size_t new_size) {
        if (new_size <= m_Vregion.GetSize())
            return false; // invalid size
        if (!(m_VPM->AttemptToExpandRight(new_size)))
            return false; // virtual page manager failed to expand
        m_Vregion.ExpandRight(new_size);
        return true;
    }

    bool PageManager::isWritable(void* addr, size_t size) const {
        PageObject* po = m_allocated_objects;
        while (po != nullptr) {
            if (addr >= po->virtual_address && (uint64_t)addr <= ((uint64_t)(po->virtual_address) + po->page_count * PAGE_SIZE)) {
                if (!(po->perms == PagePermissions::WRITE || po->perms == PagePermissions::READ_WRITE))
                    return false;
                if ((po->page_count * PAGE_SIZE) < size) {
                    size -= po->page_count * PAGE_SIZE;
                    addr = (void*)((uint64_t)addr + po->page_count * PAGE_SIZE);
                    return isWritable(addr, size);
                }
                return true;
            }
            po = po->next;
        }
        return false;
    }

    bool PageManager::isValidAllocation(void* addr, size_t size) const {
        PageObject* po = m_allocated_objects;
        while (po != nullptr) {
            if (addr >= po->virtual_address && size == (po->page_count * PAGE_SIZE))
                return true;
            po = po->next;
        }
        return false;
    }

    const VirtualRegion& PageManager::GetRegion() const {
        return m_Vregion;
    }

    bool PageManager::InsertObject(PageObject* obj) {
        if (m_allocated_object_count > 0) {
            PageObject* previous = PageObject_GetPrevious(m_allocated_objects, obj);
            if (previous == nullptr)
                return false;
            else
                previous->next = obj;
        }
        else
            m_allocated_objects = obj;
        m_allocated_object_count++;
        return true;
    }

}