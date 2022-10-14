#ifndef _STDINT_H
#define _STDINT_H

typedef unsigned char uint8_t;
typedef signed   char  int8_t;

typedef unsigned short uint16_t;
typedef          short  int16_t;
typedef unsigned int   uint32_t;
typedef          int    int32_t;

typedef unsigned long long uint64_t;
typedef          long long  int64_t;

typedef uint64_t intmax_t;

typedef uint64_t uintptr_t;
typedef  int64_t  intptr_t;

#endif /* _STDINT_H */