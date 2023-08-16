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

#include "newdelete.hpp"

#include <stdio.hpp>
#include <stdlib.h>
#include <util.h>

void* operator new(size_t size) throw() {
    if (NewDeleteInitialised()) {
        return kcalloc(DIV_ROUNDUP(size, 8), 8);
    }
    else {
        fprintf(VFS_DEBUG, "operator new attempted when unavailable. returning a null pointer.\n");
    }
    return nullptr;
}

void* operator new[](size_t size) throw() {
    if (NewDeleteInitialised()) {
        return kcalloc(DIV_ROUNDUP(size, 8), 8);
    }
    else {
        fprintf(VFS_DEBUG, "operator new[] attempted when unavailable. returning a null pointer.\n");
    }
    return nullptr;
}

void operator delete(void* p) {
    if (NewDeleteInitialised()) {
        return kfree(p);
    }
    else {
        fprintf(VFS_DEBUG, "operator delete attempted when unavailable. doing nothing.\n");
    }
}

void operator delete[](void* p) {
    if (NewDeleteInitialised()) {
        return kfree(p);
    }
    else {
        fprintf(VFS_DEBUG, "operator delete[] attempted when unavailable. doing nothing.\n");
    }
}

void operator delete(void* p, size_t) {
    if (NewDeleteInitialised()) {
        return kfree(p);
    }
    else {
        fprintf(VFS_DEBUG, "operator delete attempted when unavailable. doing nothing.\n");
    }
}

void operator delete[](void* p, size_t) {
    if (NewDeleteInitialised()) {
        return kfree(p);
    }
    else {
        fprintf(VFS_DEBUG, "operator delete[] attempted when unavailable. doing nothing.\n");
    }
}


bool g_NewDeleteInitialised = false;

bool NewDeleteInitialised() {
    return g_NewDeleteInitialised;
}

void NewDeleteInit() {
    g_NewDeleteInitialised = true;
}

void NewDeleteDestroy() {
    // TODO: check if there are any active objects that could cause issues
    g_NewDeleteInitialised = false;
}
