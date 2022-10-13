#include "vfs.h"

#include "hal.h"
#include "graphics.h"

void VFS_write(const fd_t file, const uint8_t* bytes, const size_t length) {
    switch (file) {
        case VFS_STDIN:
            break;
        case VFS_STDOUT:
        case VFS_STDERR:
            for (uint64_t i = 0; i < length; i++) {
                VGA_putc(bytes[i]);
            }
            break;
        case VFS_DEBUG:
            for (uint64_t i = 0; i < length; i++) {
                WorldOS::debug_putc(bytes[i]);
            }
            break;
        case VFS_DEBUG_AND_STDOUT:
            for (uint64_t i = 0; i < length; i++) {
                WorldOS::debug_putc(bytes[i]);
                VGA_putc(bytes[i]);
            }
            break;
        default:
            break;
    }
}