#include "panic.h"

#include <graphics.h>
#include <cstr.h>

#define OUT(x) WorldOS::GlobalBasicRenderer->Print(x)
#define CHECK(x) x == nullptr ? "Unable to print hex number." : x

void x86_64_Panic(const char* reason, x86_64_Registers* regs, const bool type) {
    using namespace WorldOS;

    GlobalBasicRenderer->ClearScreen(0xFF1A00F7 /* blue */);
    GlobalBasicRenderer->SetBackgroundColour(0xFF1A00F7 /* blue */);
    Position pos = {0,0};
    GlobalBasicRenderer->SetCursorPosition(pos);

    OUT("KERNEL PANIC!\nError Message:  ");
    OUT(reason);

    if (type /* true = interrupt */) {
        OUT("\nRAX=");
        OUT(CHECK(to_hstring(regs->RAX)));
        OUT("    RCX=");
        OUT(CHECK(to_hstring(regs->RCX)));
        OUT("    RBX=");
        OUT(CHECK(to_hstring(regs->RBX)));
        OUT("\nRDX=");
        OUT(CHECK(to_hstring(regs->RDX)));
        OUT("    RSP=");
        OUT(CHECK(to_hstring(regs->RSP)));
        OUT("    RBP=");
        OUT(CHECK(to_hstring(regs->RBP)));
        OUT("\nRSI=");
        OUT(CHECK(to_hstring(regs->RSI)));
        OUT("    RDI=");
        OUT(CHECK(to_hstring(regs->RDI)));
        OUT("    R8=");
        OUT(CHECK(to_hstring(regs->R8)));
        OUT("\nR9=");
        OUT(CHECK(to_hstring(regs->R9)));
        OUT("    R10=");
        OUT(CHECK(to_hstring(regs->R10)));
        OUT("    R11=");
        OUT(CHECK(to_hstring(regs->R11)));
        OUT("\nR12=");
        OUT(CHECK(to_hstring(regs->R12)));
        OUT("    R13=");
        OUT(CHECK(to_hstring(regs->R13)));
        OUT("    R14=");
        OUT(CHECK(to_hstring(regs->R14)));
        OUT("\nR15=");
        OUT(CHECK(to_hstring(regs->R15)));
        OUT("    RIP=");
        OUT(CHECK(to_hstring(regs->rip)));
        OUT("    RFLAGS=");
        OUT(CHECK(to_hstring(regs->rflags)));
        OUT("\nCS=");
        OUT(CHECK(to_hstring(regs->cs)));
        OUT("    DS=");
        OUT(CHECK(to_hstring(regs->ds)));
        OUT("    SS=");
        OUT(CHECK(to_hstring(regs->ss)));
        OUT("\nINTERRUPT=");
        OUT(CHECK(to_hstring(regs->interrupt)));
        if (regs->error != 0) {
            OUT("    ERROR CODE=");
            OUT(CHECK(to_hstring(regs->error)));
        }
    } else {
        OUT("\nNo extra details are shown when type isn't Interrupt/Exception");
    }

    while (true) {
        // just hang
    }
}

#undef OUT