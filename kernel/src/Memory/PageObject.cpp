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

#include "PageObject.hpp"

#include <Data-structures/Bitmap.hpp>

#include <util.h>

void PageObject_SetFlag(PageObject*& obj, uint64_t flag) {
    obj->flags |= flag;
}

void PageObject_UnsetFlag(PageObject*& obj, uint64_t flag) {
    obj->flags &= ~flag;
}

PageObject* PageObject_GetPrevious(PageObject* root, PageObject* current) {
    if (root == nullptr || root == current)
        return nullptr;
    if (root->next == nullptr)
        return root;
    if (root->next == current)
        return root;
    return PageObject_GetPrevious(root->next, current);
}


PageObject g_PageObjectPool[PAGE_OBJECT_POOL_SIZE];

Bitmap g_PageObjectPool_Bitmap;

uint8_t g_PageObjectPool_Bitmap_Buffer[PAGE_OBJECT_POOL_SIZE / 8];

uint64_t g_PageObjectPool_UsedCount = 0;

bool g_PageObjectPool_HasBeenInitialised = false;

void PageObjectPool_Init() {
    g_PageObjectPool_Bitmap.SetBuffer(g_PageObjectPool_Bitmap_Buffer);
    g_PageObjectPool_Bitmap.SetSize(PAGE_OBJECT_POOL_SIZE / 8);
    memset(g_PageObjectPool, 0, PAGE_OBJECT_POOL_SIZE / 8);
    g_PageObjectPool_UsedCount = 0;
    g_PageObjectPool_HasBeenInitialised = true;
}

void PageObjectPool_Destroy() {
    for (uint64_t i = 0; i <= PAGE_OBJECT_POOL_SIZE; i++) {
        if (g_PageObjectPool_Bitmap[i] == true) {
            g_PageObjectPool_Bitmap.Set(i, false);
            g_PageObjectPool_UsedCount--;
        }
    }
    g_PageObjectPool_Bitmap.~Bitmap();
    g_PageObjectPool_HasBeenInitialised = false;
}

bool PageObjectPool_HasBeenInitialised() {
    return g_PageObjectPool_HasBeenInitialised;
}

bool PageObjectPool_IsInPool(PageObject* obj) {
    return (((uint64_t)obj >= (uint64_t)g_PageObjectPool) && ((uint64_t)obj < ((uint64_t)g_PageObjectPool + PAGE_OBJECT_POOL_SIZE * sizeof(PageObject))));
}

PageObject* PageObjectPool_Allocate() {
    if (!g_PageObjectPool_HasBeenInitialised)
        return nullptr;
    if (g_PageObjectPool_UsedCount == PAGE_OBJECT_POOL_SIZE) return nullptr;
    for (uint64_t i = 0; i <= PAGE_OBJECT_POOL_SIZE; i++) {
        if (g_PageObjectPool_Bitmap[i] == 0) {
            g_PageObjectPool_Bitmap.Set(i, true);
            g_PageObjectPool_UsedCount++;
            return &(g_PageObjectPool[i]);
        }
    }
    return nullptr;
}

void PageObjectPool_Free(PageObject* obj) {
    if (!g_PageObjectPool_HasBeenInitialised)
        return;
    if (g_PageObjectPool_UsedCount == 0) return;
    for (uint64_t i = 0; i <= PAGE_OBJECT_POOL_SIZE; i++) {
        if ((uint64_t)obj == (uint64_t)(&(g_PageObjectPool[i]))) {
            g_PageObjectPool_Bitmap.Set(i, false);
            g_PageObjectPool_UsedCount--;
            return;
        }
    }
}
