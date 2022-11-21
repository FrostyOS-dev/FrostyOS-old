#ifndef _KERNEL_UTIL_H
#define _KERNEL_UTIL_H

#include "stdint.h"
#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#endif

void* memset(void* dst, const uint8_t value, const size_t n);
void* memcpy(void* dst, const void* src, const size_t n);
void* memmove(void* dst, const void* src, const size_t n);

/*
Uses 64-bit operations to quick fill a buffer.
dst is where you write to
value is the value you want to write
n is the amount of times you want to write that value
*/
void fast_memset(void* dst, const uint64_t value, const size_t n);

/*
Uses 64-bit operations to quick copy between buffers.
dst is where you write to
src is the value you want to copy from
n is the amount of data you want to copy, and must be a multiple of 8. If it isn't, it will skip the extra, so it is a multiple of 8.
*/
void* fast_memcpy(void* dst, const void* src, const size_t n);

/*
Uses 64-bit operations to quick copy between buffers.
dst is where you write to
src is the value you want to copy from
n is the amount of data you want to copy, and must be a multiple of 8. If it isn't, it will skip the extra, so it is a multiple of 8.
*/
void* fast_memmove(void* dst, const void* src, const size_t n);

uint8_t memcmp(const void* s1, const void* s2, const size_t size);

#define FLAG_SET(x, flag) x |= (flag)
#define FLAG_UNSET(x, flag) x &= ~(flag)

#ifdef __cplusplus
}
#endif

#endif /*_KERNEL_UTIL_H */