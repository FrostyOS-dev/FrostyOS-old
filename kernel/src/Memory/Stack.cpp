/*
Copyright (©) 2024  Frosty515

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

#include "Stack.hpp"
#include "PageManager.hpp"

#include <util.h>

void* CreateKernelStack() {
    void* stack_range_start = g_KPM->ReservePages(KERNEL_STACK_SIZE / PAGE_SIZE + 2, PagePermissions::READ_WRITE);
    return g_KPM->AllocatePages(KERNEL_STACK_SIZE / PAGE_SIZE, PagePermissions::READ_WRITE, (void*)((uint64_t)stack_range_start + PAGE_SIZE));
}

void DestroyKernelStack(void* stack) {
    g_KPM->FreePage((void*)((uint64_t)stack - PAGE_SIZE));
    g_KPM->FreePages(stack);
    g_KPM->FreePage((void*)((uint64_t)stack + KERNEL_STACK_SIZE));
}
