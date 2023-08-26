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

    PageManager::PageManager() : m_allocated_objects(nullptr), m_allocated_object_count(0), m_Vregion(), m_VPM(), m_mode(false), m_page_object_pool_used(false) {
        
    }

    PageManager::PageManager(const VirtualRegion& region, VirtualPageManager* VPM, bool mode) : m_allocated_objects(nullptr), m_allocated_object_count(0), m_Vregion(region), m_VPM(VPM), m_mode(mode), m_page_object_pool_used(false) {
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
    }

    void PageManager::InitPageManager(const VirtualRegion& region, VirtualPageManager* VPM, bool mode) {
        m_allocated_objects = nullptr;
        m_allocated_object_count = 0;
        m_Vregion = region;
        m_VPM = VPM;
        m_mode = mode;
        m_page_object_pool_used = false;
        if (!PageObjectPool_HasBeenInitialised())
            PageObjectPool_Init();
    }

    void* PageManager::AllocatePage() {
        if (m_mode)
            return nullptr;
        void* phys_addr = g_PPFA->AllocatePage();
        if (phys_addr == nullptr)
            return nullptr;
        void* virt_addr = m_VPM->AllocatePage();
        if (virt_addr == nullptr) {
            g_PPFA->FreePage(phys_addr);
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
            g_PPFA->FreePage(phys_addr);
            m_VPM->UnallocatePage(virt_addr);
            return nullptr;
        }
        PageObject_SetFlag(po, PO_ALLOCATED);
        po->physical_address = phys_addr;
        po->virtual_address = virt_addr;
        po->page_count = 1;
        if (m_allocated_object_count > 0) {
            PageObject* previous = PageObject_GetPrevious(m_allocated_objects, po);
            if (previous == nullptr) {
                if (PageObjectPool_IsInPool(po))
                    PageObjectPool_Free(po);
                else if (NewDeleteInitialised())
                    delete po;
                g_PPFA->FreePage(phys_addr);
                m_VPM->UnallocatePage(virt_addr);
                return nullptr;
            }
            else
                previous->next = po;
        }
        else
            m_allocated_objects = po;
        m_allocated_object_count++;
        if (!m_mode /* supervisor */)
            MapPage(phys_addr, virt_addr, 0x8000003); // Read/Write, No execute, Present
        return virt_addr;
    }

    void* PageManager::AllocatePages(uint64_t count) {
        if (m_mode)
            return nullptr;
        if (count == 1)
            return AllocatePage();
        void* phys_addr = g_PPFA->AllocatePages(count);
        if (phys_addr == nullptr)
            return nullptr;
        void* virt_addr = m_VPM->AllocatePages(count);
        if (virt_addr == nullptr) {
            g_PPFA->FreePages(phys_addr, count);
            fprintf(VFS_DEBUG, "virt_addr == nullptr\n");
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
            g_PPFA->FreePages(phys_addr, count);
            m_VPM->UnallocatePages(virt_addr, count);
            return nullptr;
        }
        PageObject_SetFlag(po, PO_ALLOCATED);
        po->physical_address = phys_addr;
        po->virtual_address = virt_addr;
        po->page_count = count;
        if (m_allocated_object_count > 0) {
            PageObject* previous = PageObject_GetPrevious(m_allocated_objects, po);
            if (previous == nullptr) {
                if (PageObjectPool_IsInPool(po))
                    PageObjectPool_Free(po);
                else if (NewDeleteInitialised())
                    delete po;
                g_PPFA->FreePages(phys_addr, count);
                m_VPM->UnallocatePages(virt_addr, count);
                return nullptr;
            }
            else
                previous->next = po;
        }
        else
            m_allocated_objects = po;
        m_allocated_object_count++;
        if (!m_mode /* supervisor */) {
            for (uint64_t i = 0; i < count; i++)
                MapPage(phys_addr + i * 4096, virt_addr + i * 4096, 0x8000003); // Read/Write, No execute, Present
        }
        return virt_addr;
    }

    void PageManager::FreePage(void* addr) {
        PageObject* po = m_allocated_objects;
        while (po != nullptr) {
            if (po->virtual_address == addr && po->page_count == 1) {
                g_PPFA->FreePage(po->physical_address);
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
        return; // ignore invalid address
    }

    void PageManager::FreePages(void* addr) {
        PageObject* po = m_allocated_objects;
        while (po != nullptr) {
            if (po->virtual_address == addr && po->page_count > 1) {
                g_PPFA->FreePages(po->physical_address, po->page_count);
                m_VPM->UnallocatePages(addr, po->page_count);
                for (uint64_t i = 0; i < po->page_count; i++)
                    UnmapPage((void*)((uint64_t)addr + i * 0x1000));
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
        return; // ignore invalid address
    }

}