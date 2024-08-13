#include "UserPanic.hpp"
#include "hal.hpp"

#include <stdio.h>

extern "C" void __attribute__((noreturn)) UserPanic(const char* reason) {
    puts("KERNEL PANIC!\n");
    puts(reason);
    // Shutdown();
    while (1) {
    }
}
