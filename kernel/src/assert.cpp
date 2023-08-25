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

#include "assert.h"

#include <HAL/hal.hpp>

#include <stdio.hpp>

extern void __assert_fail(const char* assertion, const char* file, unsigned int line, const char* function) {
    // FIXME: change this function call so it outputs to stderr once that works properly
    fprintf(VFS_DEBUG, "Assertion failed: \"%s\", file %s, line %u, function \"%s\"\n", assertion, file, line, function);
    PANIC("Assertion failed. See debug log for more info.");
}