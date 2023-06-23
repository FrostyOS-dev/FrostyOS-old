#include "timer.hpp"

#include <arch/x86_64/PIT.hpp>

#include <util.h>

void HAL_TimerInit() {
    x86_64_PIT_Init();
    x86_64_PIT_SetDivisor(11932 /* just slightly under 100 Hz */);
}

void sleep(uint64_t ms) {
    x86_64_PIT_SetTicks(0);
    ms = ALIGN_UP(ms, 10);

    while (x86_64_PIT_GetTicks() < ms) {}
        //__asm__ volatile ("hlt"); // does the same as having an empty loop except the CPU uses less power
}
