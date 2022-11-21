#include "panic.hpp"

#include <stdio.hpp>

#include <HAL/graphics.hpp>

void x86_64_Panic(const char* reason, x86_64_Registers* regs, const bool type) {

    VGA_ClearScreen(0xFF1A00F7 /* blue */);
    VGA_SetBackgroundColour(0xFF1A00F7 /* blue */);
    Position pos = {0,0};
    VGA_SetCursorPosition(pos);

    fprintf(VFS_DEBUG_AND_STDOUT, "KERNEL PANIC!\nError Message:  %s", reason);

    if (type /* true = interrupt */) {
        fprintf(VFS_DEBUG_AND_STDOUT, "\nRAX=%x    RCX=%x    RBX=%x\nRDX=%x    RSP=%x    RBP=%x\nRSI=%x    RDI=%x    R8=%x\nR9=%x    R10=%x    R11=%x\nR12=%x    R13=%x    R14=%x\nR15=%x    RIP=%x    RFLAGS=%x", regs->RAX, regs->RCX, regs->RBX, regs->RDX, regs->RSP, regs->RBP, regs->RSI, regs->RDI, regs->R8, regs->R9, regs->R10, regs->R11, regs->R12, regs->R13, regs->R14, regs->R15, regs->rip, regs->rflags);
        fprintf(VFS_DEBUG_AND_STDOUT, "\nCS=%x    DS=%x    SS=%x\nINTERRUPT=%x", regs->cs, regs->ds, regs->ss, regs->interrupt);
        if (regs->error != 0) {
            fprintf(VFS_DEBUG_AND_STDOUT, "    ERROR CODE=%x", regs->error);
        }
    } else {
        fprintf(VFS_DEBUG_AND_STDOUT, "\nNo extra details are shown when type isn't Interrupt/Exception");
    }

    while (true) {
        // just hang
    }
}

#undef OUT