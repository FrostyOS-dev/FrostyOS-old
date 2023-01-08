#include "util.h"

void* memset(void* dst, const uint8_t value, const size_t n) {
    uint8_t* d = (uint8_t*) dst;

    for (size_t i = 0; i < n; i++) {
        d[i] = value;
    }

    return dst;
}

void* memcpy(void* dst, const void* src, const size_t n) {
    uint8_t* d = (uint8_t*) dst;
    const uint8_t* s = (const uint8_t*) src;

    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }

    return dst;
}

void* memmove(void* dst, const void* src, const size_t n) {
    // OK, since we know that memcpy copies forwards
    if (dst < src) {
        return memcpy(dst, src, n);
    }

    uint8_t *d = (uint8_t*) dst;
    const uint8_t *s = (const uint8_t*) src;

    for (size_t i = n; i > 0; i--) {
        d[i - 1] = s[i - 1];
    }

    return dst;
}

/*
Uses 64-bit operations to quick fill a buffer.
dst is where you write to
value is the value you want to write
n is the amount of times you want to write that value
*/
void fast_memset(void* dst, const uint64_t value, const size_t n) {
    uint64_t* d = (uint64_t*)((uint8_t*)dst);

    for (size_t i = 0; i < n; i++) {
        d[i] = value;
    }
}

/*
Uses 64-bit operations to quick copy between buffers.
dst is where you write to
src is the value you want to copy from
n is the amount of data you want to copy, and must be a multiple of 8. If it isn't, it will skip the extra, so it is a multiple of 8.
*/
void* fast_memcpy(void* dst, const void* src, const size_t n) {
    uint64_t* d = (uint64_t*)((uint8_t*)dst);
    const uint64_t* s = (const uint64_t*)((const uint8_t*)src);

    for (size_t i = 0; i < (n / 8); i++) {
        d[i] = s[i];
    }

    return dst;
}

/*
Uses 64-bit operations to quick copy between buffers.
dst is where you write to
src is the value you want to copy from
n is the amount of data you want to copy, and must be a multiple of 8. If it isn't, it will skip the extra, so it is a multiple of 8.
*/
void* fast_memmove(void* dst, const void* src, const size_t n) {
    // OK, since we know that memcpy copies forwards
    if (dst < src) {
        return fast_memcpy(dst, src, n);
    }

    uint64_t* d = (uint64_t*)((uint8_t*)dst);
    const uint64_t* s = (const uint64_t*)((const uint8_t*)src);

    for (size_t i = (n/8); i > 0; i--) {
        d[i - 1] = s[i - 1];
    }

    return dst;
}

int memcmp(const void* s1, const void* s2, const size_t n) {
    const uint8_t* src1 = (const uint8_t*)s1;
    const uint8_t* src2 = (const uint8_t*)s2;

    for (size_t i = 0; i < n; i++) {
        if (src1[i] != src2[i]) {
            return src1[i] > src2[i] ? 1 : -1;
        }
    }

    return 0;
}