#include "newdelete.h"

/*
Source code is in C++ even though the header is in C because of the specification
*/

#include <stdio.hpp>

void* operator new(size_t size) {
    fprintf(VFS_DEBUG, "operator new attempted when unavailable. returning a null pointer.");
    return nullptr;
}

void* operator new[](size_t size) {
    fprintf(VFS_DEBUG, "operator new[] attempted when unavailable. returning a null pointer.");
    return nullptr;
}

void operator delete(void* p) {
    fprintf(VFS_DEBUG, "operator delete attempted when unavailable. doing nothing.");
}

void operator delete[](void* p) {
    fprintf(VFS_DEBUG, "operator delete[] attempted when unavailable. doing nothing.");
}
