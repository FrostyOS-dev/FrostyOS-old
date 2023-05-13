#ifndef _PAGE_MANAGER_H
#define _PAGE_MANAGER_H

#include <stdint.h>
#include <stddef.h>

#include "PageObject.hpp"
#include "Memory.hpp"

namespace WorldOS {

    class PageManager {
    public:
        PageManager();
        PageManager(void* virtual_region_start, size_t virtual_region_size, bool mode = false); // mode is false for supervisor and true for user
        ~PageManager();

        void InitPageManager(void* virtual_region_start, size_t virtual_region_size, bool mode = false); // Extra function for later initialisation. mode is false for supervisor and true for user
        
        void* AllocatePage();
        void* AllocatePages(uint64_t count);

        void FreePage(void* addr);
        void FreePages(void* addr);

    private:
        PageObject* m_allocated_objects;
        uint64_t m_allocated_object_count;
        
        void* m_Vregion_start;
        size_t m_Vregion_size;

        bool m_mode;

        bool m_page_object_pool_used;
    };

    extern PageManager* g_KPM;

}

#endif /* _PAGE_MANAGER_H */