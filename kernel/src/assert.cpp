#include "assert.h"

#include <HAL/hal.hpp>

#include <stdio.hpp>

extern void __assert_fail(const char* assertion, const char* file, unsigned int line, const char* function) {
    // FIXME: change this function call so it outputs to stderr once that works properly
    fprintf(VFS_DEBUG, "Assertion failed: \"%s\", file %s, line %u, function \"%s\"\n", assertion, file, line, function);
    WorldOS::Panic("Assertion failed. See debug log for more info.", nullptr, false);
}