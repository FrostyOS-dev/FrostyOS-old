#ifndef _KERNEL_STDLIB_H
#define _KERNEL_STDLIB_H

#ifdef __cplusplus
extern "C" {
#endif


#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0
#define NULL ((void*)0)


typedef unsigned long size_t;


int atoi(const char* str);
long atol(const char* str);

long strtol(const char* str);
unsigned long strtoul(const char* str);

#ifdef __cplusplus
}
#endif

#endif /* _KERNEL_STDLIB_H */