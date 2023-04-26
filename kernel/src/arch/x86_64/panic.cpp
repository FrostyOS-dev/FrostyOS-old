#include "panic.hpp"

#include <stdio.hpp>

#include <HAL/graphics.hpp>

void x86_64_Panic(const char* reason, x86_64_Registers* regs, const bool type) {

    // Output all to debug first

    fprintf(VFS_DEBUG, "KERNEL PANIC!\nError Message:  %s", reason);

    if (type /* true = interrupt */) {
        fprintf(VFS_DEBUG, "\nRAX=%lx    RCX=%lx    RBX=%lx\nRDX=%lx    RSP=%lx    RBP=%lx\nRSI=%lx    RDI=%lx    R8=%lx\nR9=%lx    R10=%lx    R11=%lx\nR12=%lx    R13=%lx    R14=%lx\nR15=%lx    RIP=%lx    RFLAGS=%lx", regs->RAX, regs->RCX, regs->RBX, regs->RDX, regs->RSP, regs->RBP, regs->RSI, regs->RDI, regs->R8, regs->R9, regs->R10, regs->R11, regs->R12, regs->R13, regs->R14, regs->R15, regs->rip, regs->rflags);
        fprintf(VFS_DEBUG, "\nCS=%x    DS=%x    SS=%x\nINTERRUPT=%x", regs->cs, regs->ds, regs->ss, regs->interrupt);
        if (regs->error != 0) {
            fprintf(VFS_DEBUG, "    ERROR CODE=%x", regs->error);
        }
        if (regs->interrupt == 0xE /* Page Fault */)
            fprintf(VFS_DEBUG, "\nCR2=%lx\n", regs->CR2);
    } else {
        fprintf(VFS_DEBUG, "No extra details are shown when type isn't Interrupt/Exception");
    }

    // Output all to stdout after in case framebuffer writes cause a page fault

    VGA_ClearScreen(0xFF1A00F7 /* blue */);
    VGA_SetBackgroundColour(0xFF1A00F7 /* blue */);
    Position pos = {0,0};
    VGA_SetCursorPosition(pos);

    fprintf(VFS_STDOUT, "KERNEL PANIC!\nError Message:  %s", reason);

    if (type /* true = interrupt */) {
        fprintf(VFS_STDOUT, "\nRAX=%lx    RCX=%lx    RBX=%lx\nRDX=%lx    RSP=%lx    RBP=%lx\nRSI=%lx    RDI=%lx    R8=%lx\nR9=%lx    R10=%lx    R11=%lx\nR12=%lx    R13=%lx    R14=%lx\nR15=%lx    RIP=%lx    RFLAGS=%lx", regs->RAX, regs->RCX, regs->RBX, regs->RDX, regs->RSP, regs->RBP, regs->RSI, regs->RDI, regs->R8, regs->R9, regs->R10, regs->R11, regs->R12, regs->R13, regs->R14, regs->R15, regs->rip, regs->rflags);
        fprintf(VFS_STDOUT, "\nCS=%x    DS=%x    SS=%x\nINTERRUPT=%x", regs->cs, regs->ds, regs->ss, regs->interrupt);
        if (regs->error != 0) {
            fprintf(VFS_STDOUT, "    ERROR CODE=%x", regs->error);
        }
        if (regs->interrupt == 0xE /* Page Fault */)
            fprintf(VFS_STDOUT, "\nCR2=%lx\n", regs->CR2);
    } else {
        fprintf(VFS_STDOUT, "No extra details are shown when type isn't Interrupt/Exception");
    }

    while (true) {
        // just hang
        __asm__ volatile ("hlt");
    }
}

#undef OUT