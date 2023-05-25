#ifndef _KERNEL_PAGE_OBJECT_HPP
#define _KERNEL_PAGE_OBJECT_HPP

#include <stdint.h>

namespace WorldOS {

    struct PageObject {
        void* physical_address;
        void* virtual_address;
        uint64_t page_count;
        uint64_t flags;

        PageObject* next;
    };

    enum PageObjectFlags {
        PO_USER       = 0b00001, // invert bit for supervisor
        PO_RESERVED   = 0b00010, // invert bit for not reserved
        PO_ALLOCATED  = 0b00100, // invert bit for free
        PO_INUSE      = 0b01000, // invert bit for unused
        PO_STANDBY    = 0b10000  // invert bit for not standby
    };

    void PageObject_SetFlag(PageObject*& obj, uint64_t flag);
    void PageObject_UnsetFlag(PageObject*& obj, uint64_t flag);
    PageObject* PageObject_GetPrevious(PageObject* root, PageObject* current);


    /* Page Object Pool stuff */

    #define PAGE_OBJECT_POOL_SIZE 64

    extern PageObject g_PageObjectPool[PAGE_OBJECT_POOL_SIZE];

    void PageObjectPool_Init();
    void PageObjectPool_Destroy();

    bool PageObjectPool_HasBeenInitialised();
    bool PageObjectPool_IsInPool(PageObject* obj);

    PageObject* PageObjectPool_Allocate();

    void PageObjectPool_Free(PageObject* obj);

};

#endif /* _KERNEL_PAGE_OBJECT_HPP */