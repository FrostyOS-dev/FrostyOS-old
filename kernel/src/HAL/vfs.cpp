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

#include "vfs.hpp"

#include <tty/TTY.hpp>

#include <arch/x86_64/E9.h>

void VFS_write(const fd_t file, const uint8_t* bytes, const size_t length) {
    switch (file) {
        case VFS_STDIN:
            break;
        case VFS_STDOUT:
        case VFS_STDERR:
            for (uint64_t i = 0; i < length; i++)
                g_CurrentTTY->putc(bytes[i]);
            break;
        case VFS_DEBUG:
            for (uint64_t i = 0; i < length; i++)
                x86_64_debug_putc(bytes[i]);
            break;
        case VFS_DEBUG_AND_STDOUT:
            for (uint64_t i = 0; i < length; i++) {
                x86_64_debug_putc(bytes[i]);
                g_CurrentTTY->putc(bytes[i]);
            }
            break;
        default:
            break;
    }
}