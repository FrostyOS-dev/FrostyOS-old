# Changes

## Latest Changes - 26/04/2023

- Added PageObject system
- Added Full kernel PageManager
- Fixed up stack so it is page aligned
- Fixed up PageTableManager so framebuffer, stack and ACPI stuff are mapped
- Stopped PageTableManager from mapping entire first GiB of memory
- Moved I/O implementations into separate NASM file
- Added GetCR2 function
- Added NX/XD check
- stopped `InitKernelStack` from enabling/disabling interrupts
- Now using QEMU64 CPU
- Enabled KVM support in QEMU
- Added a map page function that does not flush the TLB
- Changed kernel mapping to not flush the TLB
- Fixed page tables
- Added HHDM address support in entry point and kernel main
- Added kernel end ELF symbol
- Fixed VirtualPageManager
- Moved paging init into its own file
- Removed PageTableManager
- Added x86_64_cpuid function which supports setting and returning eax, ebx, ecx and edx
- Added early entry file for setting up the stack
- Added `uint64_t` casts to `KiB`, `MiB` and `GiB` macros
- Added relevant casts to `UINT64_MAX`, `INT64_MAX`, `INT64_MIN` and `UINT32_MAX` macros
- Fixed objdump script so it dumps in Intel syntax

## 08/03/2023

- Added functions for interacting with multiple pages to PMM
- Fixed up some PMM functions

## 05/03/2023

- Added PIT timer with full timer interface
- Fixed qemu run command line.
- Moved Virtual and physical page managers out of arch/x86_64 folder

## 27/02/2023

- Fixed interrupt returning and ISR registering
- Removed nasm building as it is unnecessary
- Made `run-utils/addr2line.sh` executable (unix filesystems only)

## 25/02/2023

- Many Virtual Page Manager improvements (nearly done)
- Added custom kernel stack
- Various refactoring and cleanup
- Added Linked List data structure with node pool (same as AVL tree)

## 09/01/2023

- Added Node pool for AVL Tree in case MM isn't active
- Added utility for new and delete
- Improved new and delete so they can check what they should do
- renamed `newdelete.h` to `newdelete.hpp`
- make toolchain check for nasm correctly

## 08/01/2023

- re-arranged folder layout for kernel source code
- Started adding AVL Trees for MM
- adjusted folder structure for OVMF
- Added temperary new/delete (doesn't actually do anything except make the compiler not complain)
- added mkgpt building

## 25/11/2022

- Switching to C style kernel main, instead of a class function
- Renamed `main.cpp` to `entry.cpp`
- Added Build and run tasks to VS Code config
- Fixed a spelling mistake in `stdbool.h`

## 21/11/2022

- Renamed all C source files to .c instead of .cpp
- Renamed all C++ headers to .hpp instead of .h

## 20/11/2022

- Updated boot partition to FAT32
- Added physical memory manager
- Graphics updates
- Bitmap updates
- Tweaked entry point
- Moved main util into C++ file
- IRQ updates
- Moved PIC files to interrupts directory
- Added full support for custom ISRs
- Started adding paging support
- Added self-building toolchain and nasm

## 19/10/2022

- Renamed 'Readme.md' to 'README.md'
- Updated Makefile
- Updated `README.md`
- Added this file

## 15/10/2022

- Renamed 'wos-stdint.h' to 'stdint.h'
- Renamed 'wos-stddef.h' to 'stddef.h'
- Renamed 'wos-stdarg.h' to 'stdarg.h'

## 13/10/2022

- Updated `README.md`
- Fixed panic screen
- Added IRQ and Legacy PIC support
- Added basic printing functions to `stdio.h`
- Added debug printing for QEMU
- Added variable argument support
- Refactored graphics into HAL
- Added basic VFS for `fprintf` & etc funtions (the VFS only supports 5 modes which can be found in `src/kernel/HAL/vfs.h`)
- Removed SSE instructions from kernel
- Turned on optimizations for kernel
- fixed IO operations
- Updated Makefile
