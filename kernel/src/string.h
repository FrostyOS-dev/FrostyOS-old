#ifndef _KERNEL_CSTR_H
#define _KERNEL_CSTR_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

// memcpy, memset, memcmp and memmove can be found in util.h

size_t strlen(const char* str);

char* strcpy(char* dst, const char* src);
char* strncpy(char* dst, const char* src, size_t n);

int strcmp(const char* str1, const char* str2);
int strncmp(const char* str1, const char* str2, size_t n);

#ifdef __cplusplus
}
#endif

#endif /* _KERNEL_CSTR_H */