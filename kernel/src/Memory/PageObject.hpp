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

#ifndef _KERNEL_PAGE_OBJECT_HPP
#define _KERNEL_PAGE_OBJECT_HPP

#include <stdint.h>

enum class PagePermissions;

struct PageObject {
    void* virtual_address;
    uint64_t page_count;
    uint64_t flags;
    PagePermissions perms;

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

#endif /* _KERNEL_PAGE_OBJECT_HPP */