#include "newdelete.hpp"
#include "kmalloc.hpp"

#include <stdio.hpp>

void* operator new(size_t size) throw() {
    if (NewDeleteInitialised()) {
        return kcalloc(size);
    }
    else {
        fprintf(VFS_DEBUG, "operator new attempted when unavailable. returning a null pointer.\n");
    }
    return nullptr;
}

void* operator new[](size_t size) throw() {
    if (NewDeleteInitialised()) {
        return kcalloc(size);
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
