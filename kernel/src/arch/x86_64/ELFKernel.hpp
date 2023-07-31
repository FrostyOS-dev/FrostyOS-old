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

#ifndef _KERNEL_X86_64_ELF_KERNEL_HPP
#define _KERNEL_X86_64_ELF_KERNEL_HPP

#include "ELFSymbols.h"

#include <stddef.h>

namespace x86_64_WorldOS {
    bool MapKernel(void* kernel_phys, void* kernel_virt, size_t kernel_size);
}

#endif /* _KERNEL_X86_64_ELF_KERNEL_HPP */