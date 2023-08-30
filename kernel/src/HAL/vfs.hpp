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

#ifndef _KERNEL_HAL_VFS_HPP
#define _KERNEL_HAL_VFS_HPP

#include <stdint.h>
#include <stddef.h>

extern "C" {

typedef uint8_t fd_t;

}

void VFS_write(const fd_t file, const uint8_t* bytes, const size_t length);

#endif /* _KERNEL_HAL_VFS_HPP */