#ifndef _KERNEL_HAL_HPP
#define _KERNEL_HAL_HPP

#include <arch/x86_64/panic.hpp>

#include <arch/x86_64/Graphics/graphics-defs.h>

#include <stdint.h>
#include <Memory/Memory.hpp>

namespace WorldOS {

    // Get the essentials running. This is (in order): GDT, IDT, ISR, FPU, Basic VGA, IRQ, PIT Timer, PMM, Kernel Paging maps, KVPM.
    void HAL_EarlyInit(MemoryMapEntry** MemoryMap, uint64_t MMEntryCount, uint64_t kernel_virtual, uint64_t kernel_physical, uint64_t kernel_size, uint64_t HHDM_start, const FrameBuffer& fb);

    // Initialise basic less-essential drivers. This is (in order): RSDP, XSDT. MUST BE CALLED AFTER KPM AND HEAP ARE READY.
    void HAL_Stage2(void* RSDP);

    // reason = message to display, regs = registers at the time of error, type = the type of error (true for interrupt and false for other)
    inline void Panic(const char* reason, x86_64_Interrupt_Registers* regs, const bool type = false) { x86_64_Panic(reason, regs, type); }
}

#endif /* _KERNEL_HAL_HPP */