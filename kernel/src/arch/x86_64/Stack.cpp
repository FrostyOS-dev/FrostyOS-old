/*
Copyright (©) 2022-2024  Frosty515

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

#include "ELFSymbols.hpp"
#include "Stack.hpp"

#include "Memory/PagingUtil.hpp"

#include <stdint.h>
#include <stdio.h>

#include <Memory/PageManager.hpp>

extern "C" {
unsigned char __attribute__((aligned(0x1000))) kernel_stack[INITIAL_KERNEL_STACK_SIZE] = {0};
unsigned long int kernel_stack_size = INITIAL_KERNEL_STACK_SIZE;
}

struct stack_frame {
    stack_frame* RBP;
    uint64_t RIP;
} __attribute__((packed));

void x86_64_walk_stack_frames(void* RBP) {
    if (RBP == nullptr) {
        printf("[%s(%lp)] WARN: no stack frames.\n", __extension__ __PRETTY_FUNCTION__, RBP);
        return;
    }

    stack_frame* frame = (stack_frame*)RBP;

    while (isInKernelSpace(frame, sizeof(stack_frame))) {
        char const* name = nullptr;
        if (g_KernelSymbols != nullptr)
            name = g_KernelSymbols->LookupSymbol(frame->RIP);
        printf("%016lx", frame->RIP);
        if (name != nullptr)
            printf(": %s\n", name);
        else
            putc('\n');
        frame = frame->RBP;
    }
}
