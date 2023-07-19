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