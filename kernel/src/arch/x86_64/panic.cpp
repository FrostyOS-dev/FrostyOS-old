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

#include "panic.hpp"
#include "io.h"
#include "Stack.hpp"
#include "ELFSymbols.hpp"

#include <stdio.hpp>

#include <Graphics/VGA.hpp>

#include <tty/TTY.hpp>

#include <Scheduling/Scheduler.hpp>

BasicVGA* g_VGADevice;

void x86_64_SetPanicVGADevice(BasicVGA* device) {
    g_VGADevice = device;
}

void  __attribute__((noreturn)) x86_64_Panic(const char* reason, x86_64_Interrupt_Registers* regs, const bool type) {
    x86_64_DisableInterrupts();

    Scheduling::Scheduler::Stop();

    // Output all to debug first

    fprintf(VFS_DEBUG, "KERNEL PANIC!\nError Message:  %s", reason);

    if (type /* true = interrupt */) {
        fprintf(VFS_DEBUG, "\nRAX=%lx    RCX=%lx    RBX=%lx\nRDX=%lx    RSP=%lx    RBP=%lx\nRSI=%lx    RDI=%lx    R8=%lx\nR9=%lx    R10=%lx    R11=%lx\nR12=%lx    R13=%lx    R14=%lx\nR15=%lx    RIP=%lx    RFLAGS=%lx", regs->RAX, regs->RCX, regs->RBX, regs->RDX, regs->RSP, regs->RBP, regs->RSI, regs->RDI, regs->R8, regs->R9, regs->R10, regs->R11, regs->R12, regs->R13, regs->R14, regs->R15, regs->rip, regs->rflags);
        fprintf(VFS_DEBUG, "\nCS=%x    DS=%x    SS=%x\nINTERRUPT=%x", regs->cs, regs->ds, regs->ss, regs->interrupt);
        if (regs->error != 0)
            fprintf(VFS_DEBUG, "    ERROR CODE=%x\n", regs->error);
        else
            fputc(VFS_DEBUG, '\n');
        if (regs->interrupt == 0xE /* Page Fault */)
            fprintf(VFS_DEBUG, "CR2=%lx    CR3=%lx\n", regs->CR2, regs->CR3);
    }
    else
        fprintf(VFS_DEBUG, "\nNo extra details are shown when type isn't Interrupt/Exception\n");

    if (type) {
        fprintf(VFS_DEBUG, "Stack trace:\n");
        char const* name = nullptr;
        if (g_KernelSymbols != nullptr)
            name = g_KernelSymbols->LookupSymbol(regs->rip);
        fprintf(VFS_DEBUG, "%lx", regs->rip);
        if (name != nullptr)
            fprintf(VFS_DEBUG, ": %s\n", name);
        else
            fputc(VFS_DEBUG, '\n');

        x86_64_walk_stack_frames((void*)(regs->RBP));
    }

    // Output all to stdout after in case framebuffer writes cause a page fault

    if (g_VGADevice == nullptr)
        fprintf(VFS_DEBUG, "\nWARNING VGA Device unavailable.\n");
    else {
        g_VGADevice->ClearScreen(0xFF1A00F7 /* blue */);
        g_VGADevice->SetBackgroundColour(0xFF1A00F7 /* blue */);
        g_VGADevice->SetCursorPosition({0,0});
        g_VGADevice->SwapBuffers(false);

        g_CurrentTTY->SetVGADevice(g_VGADevice);

        fprintf(VFS_STDOUT, "KERNEL PANIC!\nError Message:  %s", reason);

        if (type /* true = interrupt */) {
            fprintf(VFS_STDOUT, "\nRAX=%lx    RCX=%lx    RBX=%lx\nRDX=%lx    RSP=%lx    RBP=%lx\nRSI=%lx    RDI=%lx    R8=%lx\nR9=%lx    R10=%lx    R11=%lx\nR12=%lx    R13=%lx    R14=%lx\nR15=%lx    RIP=%lx    RFLAGS=%lx", regs->RAX, regs->RCX, regs->RBX, regs->RDX, regs->RSP, regs->RBP, regs->RSI, regs->RDI, regs->R8, regs->R9, regs->R10, regs->R11, regs->R12, regs->R13, regs->R14, regs->R15, regs->rip, regs->rflags);
            fprintf(VFS_STDOUT, "\nCS=%x    DS=%x    SS=%x\nINTERRUPT=%x", regs->cs, regs->ds, regs->ss, regs->interrupt);
            if (regs->error != 0)
                fprintf(VFS_STDOUT, "    ERROR CODE=%x", regs->error);
            if (regs->interrupt == 0xE /* Page Fault */)
                fprintf(VFS_STDOUT, "\nCR2=%lx    CR3=%lx\n", regs->CR2, regs->CR3);
        }
        else
            fprintf(VFS_STDOUT, "\nNo extra details are shown when type isn't Interrupt/Exception\n");
    }

    while (true) {
        // just hang
        __asm__ volatile ("hlt");
    }
}