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

#ifndef _KERNEL_MALLOC_HPP
#define _KERNEL_MALLOC_HPP

#include <stddef.h>
#include <util.h>

void kmalloc_init();
bool IsKmallocInitialised();

void kmalloc_vmm_init();
bool IsKmallocVMMInitialised();

void kmalloc_eternal_init();

size_t kmalloc_SizeInHeap(void* ptr, size_t size);

#ifdef __cplusplus
extern "C" {
#endif

void* kcalloc(size_t num, size_t size);
void kfree(void* ptr);
void* kmalloc(size_t size);
void* krealloc(void* ptr, size_t size);

void* kcalloc_vmm(size_t num, size_t size);
void kfree_vmm(void* ptr);
void* kmalloc_vmm(size_t size);
void* krealloc_vmm(void* ptr, size_t size);

void* kmalloc_eternal(size_t size);
void* kcalloc_eternal(size_t num, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* _KERNEL_MALLOC_HPP */