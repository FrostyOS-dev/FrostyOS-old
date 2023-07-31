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

#include "TSS.hpp"
#include "Stack.h"
#include "GDT.hpp"

#include <util.h>

x86_64_TSS g_TSS;

void x86_64_TSS_Init() {
    fast_memset(&g_TSS, 0, sizeof(g_TSS) / 8);
    g_TSS.RSP[0] = (uint64_t)kernel_stack + kernel_stack_size;
    x86_64_GDT_SetTSS(&g_TSS);
}
