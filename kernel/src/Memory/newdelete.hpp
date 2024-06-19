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

#ifndef _KERNEL_NEW_DELETE_H
#define _KERNEL_NEW_DELETE_H

#include <stddef.h>

// standard new/delete functions

void* operator new(size_t size);
void* operator new[](size_t size);

void operator delete(void* p);
void operator delete[](void* p);


// following functions aren't part of the C++ ABI, but are required to fix linker errors

void operator delete(void* p, size_t);
void operator delete[](void* p, size_t);

// Placement new/delete functions

void* operator new(size_t, void* p);
void* operator new[](size_t, void* p);

void operator delete(void*, void*);
void operator delete[](void*, void*);


// utility functions

bool NewDeleteInitialised(); // checks if new and delete are properly initialised
void NewDeleteInit(); // allows new and delete to call malloc and free respectively
void NewDeleteDestroy(); // disallows new and delete to call malloc and free respectively

#endif /* _KERNEL_NEW_DELETE_H */