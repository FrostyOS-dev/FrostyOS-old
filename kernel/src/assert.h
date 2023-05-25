#ifndef _ASSERT_H
#define _ASSERT_H

#ifdef NDEBUG
#define assert(expr) expr
#else

    #define __ASSERT_FUNCTION __extension__ __PRETTY_FUNCTION__

    extern void __assert_fail(const char* assertion, const char* file, unsigned int line, const char* function);

    #if defined __cplusplus
        #define assert(expr) (static_cast<bool>(expr)	? void (0) : __assert_fail (#expr, __FILE__, __LINE__, __ASSERT_FUNCTION))
    #else
        #define assert(expr)((void) sizeof ((expr) ? 1 : 0), __extension__ ({ if (expr); else __assert_fail (#expr, __FILE__, __LINE__, __ASSERT_FUNCTION); }))
    #endif
#endif

#endif /* _ASSERT_H */