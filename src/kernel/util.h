#ifndef _KERNEL_UTIL_H
#define _KERNEL_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "stdint.h"
#include "stddef.h"

static inline void* memset(void* dst, const uint8_t value, const size_t n) {
    uint8_t* d = (uint8_t*) dst;

    for (size_t i = 0; i < n; i++) {
        d[i] = value;
    }

    return dst;
}

static inline void* memcpy(void* dst, const void* src, const size_t n) {
    uint8_t* d = (uint8_t*) dst;
    const uint8_t* s = (const uint8_t*) src;

    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }

    return dst;
}

static inline void* memmove(void* dst, const void* src, const size_t n) {
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

static inline uint8_t memcmp(const void* s1, const void* s2, const size_t size) {
    const uint8_t* src1 = (const uint8_t*)s1;
    const uint8_t* src2 = (const uint8_t*)s2;

    for (size_t i = 0; i < size; i++) {
        if (src1[i] != src2[i]) {
            return src1[i] > src2[i] ? 1 : -1;
        }
    }

    return 0;
}

#ifdef __cplusplus
} // extern "C"
#endif /* __cplusplus */

#define FLAG_SET(x, flag) x |= (flag)
#define FLAG_UNSET(x, flag) x &= ~(flag)

#endif /*_KERNEL_UTIL_H */