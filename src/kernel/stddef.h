#ifndef _WOS_STDDEF_H
#define _WOS_STDDEF_H

#include "stdint.h"

typedef uint64_t size_t;

#ifdef __cplusplus
    typedef decltype(nullptr) nullptr_t;
    #define NULL nullptr
#else
    #define NULL ((void*)0)
#endif /* __cplusplus */

#endif /* _WOS_STDDEF_H */