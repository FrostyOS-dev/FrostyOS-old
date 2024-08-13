#include "Graphics/Graphics.h"
#include "kernel.hpp"
#include <HAL/hal.hpp>

#include <stdio.h>
#include <string.h>

#define O_RDONLY	     00

#define MAP_GROWSDOWN	0x00100		/* Stack-like segment.  */
#define MAP_DENYWRITE	0x00800		/* ETXTBSY.  */
#define MAP_EXECUTABLE	0x01000		/* Mark it as an executable.  */
#define MAP_LOCKED	0x02000		/* Lock the mapping.  */
#define MAP_NORESERVE	0x04000		/* Don't check for reservations.  */
#define MAP_POPULATE	0x08000		/* Populate (prefault) pagetables.  */
#define MAP_NONBLOCK	0x10000		/* Do not block on IO.  */
#define MAP_STACK	0x20000		/* Allocation is for a stack.  */
#define MAP_HUGETLB	0x40000		/* Create huge page mapping.  */
#define MAP_SYNC	0x80000		/* Perform synchronous page faults for the mapping.  */
#define MAP_FIXED_NOREPLACE 0x100000	/* MAP_FIXED but do not unmap underlying mapping.  */

#define PROT_READ	0x1		/* Page can be read.  */
#define PROT_WRITE	0x2		/* Page can be written.  */
#define PROT_EXEC	0x4		/* Page can be executed.  */
#define PROT_NONE	0x0		/* Page can not be accessed.  */
#define PROT_GROWSDOWN	0x01000000	/* Extend change to start of growsdown vma (mprotect only).  */
#define PROT_GROWSUP	0x02000000	/* Extend change to start of growsup vma (mprotect only).  */

/* Sharing types (must choose one and only one of these).  */
#define MAP_SHARED	0x01		/* Share changes.  */
#define MAP_PRIVATE	0x02		/* Changes are private.  */
#define MAP_SHARED_VALIDATE	0x03	/* Share changes and validate extension flags.  */
#define MAP_TYPE	0x0f		/* Mask for type of mapping.  */

/* Other flags.  */
#define MAP_FIXED	0x10		/* Interpret addr exactly.  */
#define MAP_FILE	0
#ifdef __MAP_ANONYMOUS
# define MAP_ANONYMOUS	__MAP_ANONYMOUS	/* Don't use a file.  */
#else
# define MAP_ANONYMOUS	0x20		/* Don't use a file.  */
#endif
#define MAP_ANON	MAP_ANONYMOUS


int main(int argc, char** argv) {
    if (argc < 2) {
        char msg[1024]; // should never need that much
        snprintf(msg, 1023, "Usage: %s <initramfs_path>\n", argv[0]);
        __user_write(1, msg, strlen(msg));
        return 1;
    }

    int fd = __user_open(argv[1], O_RDONLY);
    if (fd < 0) {
        __user_write(1, "initramfs file open failed", 26);
        return 1;
    }

    struct stat statbuf;
    __user_fstat(fd, &statbuf);
    size_t size = statbuf.st_size;

    void* addr = __user_mmap(nullptr, ALIGN_UP(size, PAGE_SIZE), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if ((int64_t)addr < 0) {
        __user_write(1, "mmap failed", 11);
        return 1;
    }

    __user_read(fd, addr, size);

    __user_close(fd);

    KernelParams params = {
        .frameBuffer = FrameBuffer(),
        .MemoryMap = nullptr,
        .MemoryMapEntryCount = 0,
        .EFI_SYSTEM_TABLE_ADDR = nullptr,
        .kernel_physical_addr = 0,
        .kernel_virtual_addr = 0,
        .kernel_size = 0,
        .RSDP_table = nullptr,
        .hhdm_start_addr = 0,
        .initramfs_addr = addr,
        .initramfs_size = size,
    };
    StartKernel(&params);

    return 0;
}