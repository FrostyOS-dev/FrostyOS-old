#ifndef _KERNEL_HAL_VFS_HPP
#define _KERNEL_HAL_VFS_HPP

#include <stdint.h>
#include <stddef.h>

typedef uint8_t fd_t;

enum OUT_TYPES {
    VFS_STDIN            = 0,
    VFS_STDOUT           = 1,
    VFS_STDERR           = 2, // same as STDOUT for now
    VFS_DEBUG            = 3,
    VFS_DEBUG_AND_STDOUT = 4,
};

void VFS_write(const fd_t file, const uint8_t* bytes, const size_t length);

#endif /* _KERNEL_HAL_VFS_HPP */