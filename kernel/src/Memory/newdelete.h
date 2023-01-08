#ifndef _KERNEL_NEW_DELETE_H
#define _KERNEL_NEW_DELETE_H

#include <stddef.h>

void* operator new(size_t size);
void* operator new[](size_t size);

void operator delete(void* p);
void operator delete[](void* p);

// following functions aren't part of the C++ ABI, but are required to fix linker errors
inline void operator delete(void* p, size_t) { operator delete(p); };


#endif /* _KERNEL_NEW_DELETE_H */