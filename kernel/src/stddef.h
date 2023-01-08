#ifndef _WOS_STDDEF_H
#define _WOS_STDDEF_H

typedef unsigned long size_t;

#ifdef __cplusplus
    typedef decltype(nullptr) nullptr_t;
    #define NULL nullptr
#else
    #define NULL ((void*)0)
#endif /* __cplusplus */

typedef long ptrdiff_t;

typedef struct {
  long long __max_align_nonce1
      __attribute__((__aligned__(__alignof__(long long))));
  long double __max_align_nonce2
      __attribute__((__aligned__(__alignof__(long double))));
} max_align_t;

#define offsetof(t, d) __builtin_offsetof(t, d)

#endif /* _WOS_STDDEF_H */