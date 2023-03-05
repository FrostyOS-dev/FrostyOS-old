#ifndef _X86_64_PIT_HPP
#define _X86_64_PIT_HPP

#include <stdint.h>
#include <stddef.h>

// Init PIT. DO NOT run if hardware interrupts are enabled.
void x86_64_PIT_Init();

// Set Divisor of the PIT
void x86_64_PIT_SetDivisor(uint64_t div);

// Get Current Tick amount of the PIT
uint64_t x86_64_PIT_GetTicks();

// Set tick amount of the PIT
void x86_64_PIT_SetTicks(uint64_t t);

#endif /* _X86_64_PIT_HPP */