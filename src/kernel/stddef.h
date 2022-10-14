#ifndef _WOS_STDDEF_H
#define _WOS_STDDEF_H

#include "stdint.h"

typedef uint64_t size_t;

#define NULL ((void*)0)

typedef decltype(nullptr) nullptr_t;

#endif /* _WOS_STDDEF_H */