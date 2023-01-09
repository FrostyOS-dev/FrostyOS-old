#include "newdelete.hpp"

#include <stdio.hpp>

void* operator new(size_t size) {
    if (NewDeleteInitialised()) {
        #ifdef MM_FINISHED
            return kmalloc(size);
        #endif
    }
    else {
        fprintf(VFS_DEBUG, "operator new attempted when unavailable. returning a null pointer.\n");
    }
    return nullptr;
}

void* operator new[](size_t size) {
    if (NewDeleteInitialised()) {
        #ifdef MM_FINISHED
            return kmalloc(size);
        #endif
    }
    else {
        fprintf(VFS_DEBUG, "operator new[] attempted when unavailable. returning a null pointer.\n");
    }
    return nullptr;
}

void operator delete(void* p) {
    if (NewDeleteInitialised()) {
        #ifdef MM_FINISHED
            return kfree(p);
        #endif
    }
    else {
        fprintf(VFS_DEBUG, "operator delete attempted when unavailable. doing nothing.\n");
    }
}

void operator delete[](void* p) {
    if (NewDeleteInitialised()) {
        #ifdef MM_FINISHED
            return kfree(p);
        #endif
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
    fprintf(VFS_DEBUG, "new and delete have been initialised!\n");
    #ifndef MM_FINISHED
        fprintf(VFS_DEBUG, "WARNING: Memory manager incomplete. Even after initialising new and delete, they will still do nothing. This time they won't print a warning. \n");
    #endif
}

void NewDeleteDestroy() {
    // TODO: check if there are any active objects that could cause issues
    g_NewDeleteInitialised = true;
    fprintf(VFS_DEBUG, "new and delete have been un-initialised!\n");
    #ifndef MM_FINISHED
        fprintf(VFS_DEBUG, "WARNING: Memory manager incomplete. Attempting to un-initialise new and delete does essentially nothing.\n");
    #endif
}
