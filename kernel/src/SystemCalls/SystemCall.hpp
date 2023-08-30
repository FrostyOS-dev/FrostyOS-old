#ifndef _SYSTEM_CALL_HPP
#define _SYSTEM_CALL_HPP

#include <stdint.h>

typedef uint64_t (*SystemCallHandler_t)(uint64_t, uint64_t, uint64_t);

#define SYSTEM_CALL_COUNT 64

extern "C" {
    
    extern const uint64_t g_syscall_count;
    extern SystemCallHandler_t g_syscall_handlers[SYSTEM_CALL_COUNT];

}

void SystemCallInit();

#endif /* _SYSTEM_CALL_HPP */