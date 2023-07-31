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

#ifndef _KERNEL_PANIC_HPP
#define _KERNEL_PANIC_HPP

#include "interrupts/isr.hpp"

#include <Graphics/VGA.hpp>

struct x86_64_PanicArgs {
    const char* reason;
    x86_64_Interrupt_Registers* regs;
    bool type = false;
};

void x86_64_SetPanicVGADevice(BasicVGA* device);

// reason = message to display, regs = registers at the time of error, type = the type of error (true for interrupt and false for other)
void  __attribute__((noreturn)) x86_64_Panic(const char* reason, x86_64_Interrupt_Registers* regs, const bool type = false);

#endif /* _KERNEL_PANIC_HPP */