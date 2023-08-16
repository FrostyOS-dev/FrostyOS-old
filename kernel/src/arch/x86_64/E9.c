/*
Copyright (©) 2022-2023  Frosty515

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "E9.h"

#include "io.h"
#include <stdint.h>

void x86_64_debug_putc(const char c) {
#ifndef NDEBUG
    x86_64_outb(0xE9, c);
#endif
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
