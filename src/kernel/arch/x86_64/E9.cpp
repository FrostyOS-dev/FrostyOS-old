#include "E9.h"

#include "io.h"
#include <stdint.h>

void x86_64_debug_putc(const char c) {
    x86_64_outb(0xE9, c);
}

void x86_64_debug_puts(const char* str) {
    uint64_t i = 0;
    char c = str[i];
    while (c != 0) {
        x86_64_debug_putc(c);
        i++;
        c = str[i];
    }
}
