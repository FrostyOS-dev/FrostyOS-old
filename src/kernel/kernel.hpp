#ifndef _KERNEL_HPP
#define _KERNEL_HPP

#include <stdint.h>
#include <stddef.h>

#include <Memory/Memory.hpp>
#include <stdio.hpp>
#include <HAL/hal.hpp>

namespace WorldOS {

    constexpr uint64_t EARLY_STAGE     = 0x5354414745000000;
    constexpr uint64_t PRE_LOGIN_STAGE = 0x5354414745000001;
    constexpr uint64_t USER_STAGE      = 0x5354414745000002;

    struct KernelParams {
        FrameBuffer frameBuffer;
        MemoryMapEntry** MemoryMap;
        size_t MemoryMapEntryCount;
        void* EFI_SYSTEM_TABLE_ADDR;
        uint64_t kernel_physical_addr;
        uint64_t kernel_virtual_addr;
        void* RSDP_table;
    };

    class Kernel {
    public:
        Kernel(KernelParams params);
        ~Kernel();

    private:
        FrameBuffer m_InitialFrameBuffer;
        uint32_t m_fgcolour;
        uint32_t m_bgcolour;
        uint64_t m_Stage;
    };
}

#endif /* _KERNEL_HPP */