#include "PageManager.hpp"
#include "newdelete.hpp"

#include <HAL/hal.hpp>

#include <arch/x86_64/Memory/PageTableManager.hpp>

namespace WorldOS {

    PageManager::PageManager() {
        m_allocated_objects = nullptr;
        m_allocated_object_count = 0;
        m_Vregion_size = 0;
        m_Vregion_start = nullptr;
        m_mode = false;
        m_page_object_pool_used = false;
    }

    PageManager::PageManager(void* virtual_region_start, size_t virtual_region_size, bool mode) {
        InitPageManager(virtual_region_start, virtual_region_size, mode);
    }

    PageManager::~PageManager() {
        m_allocated_objects = nullptr;
        m_allocated_object_count = 0;
        m_Vregion_size = 0;
        m_Vregion_start = nullptr;


        // if page object pool was used then we call panic()
        if (m_page_object_pool_used) {
            if (m_mode)
                Panic("USER PageManager illegal destruction. PageManager cannot be destroyed if page object pool has been used.", nullptr, false);
            else
                Panic("SUPERVISOR PageManager illegal destruction. PageManager cannot be destroyed if page object pool has been used.", nullptr, false);
        }
    }

    void PageManager::InitPageManager(void* virtual_region_start, size_t virtual_region_size, bool mode) {
        m_Vregion_start = virtual_region_start;
        m_Vregion_size = virtual_region_size;
        m_mode = mode;
        if (!PageObjectPool_HasBeenInitialised())
            PageObjectPool_Init();
    }

    void PageManager::AllocatePage() {

    }

    void PageManager::AllocatePages() {

    }

    void PageManager::FreePage(void* addr) {

    }

    void PageManager::FreePages(void* addr) {

    }

}