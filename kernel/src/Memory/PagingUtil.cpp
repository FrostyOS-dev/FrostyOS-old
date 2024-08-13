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

#include "PagingUtil.hpp"

#include <util.h>

#ifdef _FROSTYOS_BUILD_TARGET_IS_KERNEL
#ifdef __x86_64__

#include <arch/x86_64/Memory/PageMapIndexer.hpp>
#include <arch/x86_64/Memory/PagingUtil.hpp>

void* to_HHDM(void* phys_addr) {
    return x86_64_to_HHDM(phys_addr);
}

void* hhdm_to_phys(void* hhdm_addr) {
    return x86_64_hhdm_to_phys(hhdm_addr);
}

bool isInKernelSpace(void *base, size_t length) {
    return x86_64_isInKernelSpace(base, length);
}

#endif /* __x86_64__ */
#elif defined(_FROSTYOS_BUILD_TARGET_IS_USERLAND)

void* to_HHDM(void* phys_addr) {
    return phys_addr;
}

#endif /* _FROSTYOS_BUILD_TARGET_IS_KERNEL */
