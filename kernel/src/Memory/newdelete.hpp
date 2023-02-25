#ifndef _KERNEL_NEW_DELETE_H
#define _KERNEL_NEW_DELETE_H

#include <stddef.h>

// standard new/delete functions

void* operator new(size_t size) throw();
void* operator new[](size_t size);

void operator delete(void* p);
void operator delete[](void* p);


// following functions aren't part of the C++ ABI, but are required to fix linker errors

void operator delete(void* p, size_t);


// utility functions

bool NewDeleteInitialised(); // checks if new and delete are properly initialised
void NewDeleteInit(); // allows new and delete to call malloc and free respectively
void NewDeleteDestroy(); // disallows new and delete to call malloc and free respectively

#endif /* _KERNEL_NEW_DELETE_H */