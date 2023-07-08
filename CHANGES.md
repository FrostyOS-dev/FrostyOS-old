# Changes

## Latest Changes - 08/07/2023

- Completely redid GDT code
- Moved GDT code out of unnecessary folder

## 07/07/2023

- Fixed RTC GetWeekDay function
- Added `limine.h` to gitignore
- Added comments to `time.h`
- Moved VGA graphics code out of arch/x86_64
- Cleaned up VGA graphics code
- Added `nm.sh` to `run-utils`
- Removed unnecessary x87 FPU code

## 30/06/2023

- Added Process and Thread classes
- Added x86_64 task switch code
- Added x86_64 register save code
- Added `x86_64_get_stack_ptr` function
- Added utility function to get a new stack (for tasks)
- Added `x86_64_Interrupt_Registers` and `x86_64_Registers` conversions
- Implemented basic kernel round-robin scheduler
- Added function call from `x86_64_PIT_Handler` to `Scheduling::Scheduler::TimerTick()`
- Changed kernel init to a 2 stage initialisation with second stage being called by the scheduler
- Renamed `LATE_STAGE` to `STAGE2`

## 28/06/2023

- Fixed a redefinition error in `HAL/time.cpp`
- Removed unnecessary includes and function calls from `StartKernel`
- Added a virtual region class
- Changed VirtualPageManager so it uses a virtual region instead of relevant class variables

## 25/06/2023

- Implemented CMOS Read and Write
- Implemented Simple RTC driver
- Started implementing POSIX time
- Renamed HAL `timer` to `time`
- Changed HAL `time` to more C based
- Cleaned up I/O code
- Added `BCD_TO_BINARY` macro
- Removed unnecessary include from `kernel.cpp`

## 23/06/2023

- Moved kernel-related parts of `Makefile` into its own `Makefile`
- Moved toolchain build into `Makefile`
- Added a patch for building a `-mno-red-zone` variant of `libgcc`
- Tidied up both `Makefile`s
- Various small code refactoring to remove most compiler warnings

## 11/06/2023

- Fixed HAL timer code so the millisecond value is rounded up correctly
- Renamed `x86_64_Registers` to `x86_64_Interrupt_Registers`
- Changed `LinkedList::insert` to `LinkedList::insertNode`
- Fixed `LinkedList::deleteNode` so it fixes the existing list correctly and so page faults are very rare
- Implemented template `SimpleLinkedList` class

## 06/06/2023

- Implemented `kcalloc`
- Changed `new` to use `kcalloc`
- Changed all object pools so they check if an object is inside the pool before deleting it
- Converted `Device` and `PCIDevice` classes to pure-virtual
- Added architecture-independent version of `get_physaddr`
- Fixed `x86_64_to_HHDM` so HHDM range is first 512GiB instead of 4GiB

## 28/05/2023

- Actually implemented `strcpy` and `strncpy`
- Changed run command line so QEMU uses the `q35` chipset
- removed unnecessary `fprintf` calls in `Memory/newdelete.cpp`
- Added Simple PCIe device detection
- Added basic `Device` and `PCIDevice` classes
- Fixed `x86_64_map_page_noflush` so it doesn't print unnecessary information
- Added `getSDTCount` function to ACPI SDT files

## 27/05/2023

- Tidied up `StartKernel`
- Changed HAL to 2-stage initialisation
- Added ACPI RSDP support
- Added ACPI XSDT support
- Fixed `x86_64_Panic` so it creates a new line before printing `No extra details are shown when type isn't Interrupt/Exception`
- Fixed assert so it adds `"` around the function name

## 26/05/2023

- Implemented LinkedListBucketHeap-based kmalloc (from the OSDev wiki)
- Fixed new and delete functions so they now work
- Fixed a bug where `LinkedList::NodePool_FreeNode` did not actually set the node passed to zero
- Fixed bug in `WorldOS::PageObject_GetPrevious` where it would return `nullptr` if the root has no next
- Fixed Bitmap constructors so they follow the C++ ABI
- Made functions in Bitmap Class const if they do not modify the variables of the class
- Added resources used
- Added copyright notice and GNU General Public License overview to this file
- Added basic assert
- Added range check for all pool-based deletions
- Added useful rounding macros to `util.h`

## 13/05/2023

- Updated toolchain to Binutils 2.40 and GCC 13.1.0
- Fixed issue with PIT IRQ handler
- Removed unused variable in PIC driver
- Fixed x86_64_IDT_SetGate function so it follows ISO C++
- Fixed VirtualPageManager so it splits free blocks correctly
- Fixed PageManager PageObject management
- Fixed Initial page mapping so all physical memory is mapped to the HHDM
- Added mapping of first 4GiB to HHDM
- Added mapping for memory map entries above 4GiB
- Added 2MiB page support

## 26/04/2023

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
- Various refactoring and clean-up
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
- Added temporary new/delete (doesn't actually do anything except make the compiler not complain)
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
- Added basic VFS for `fprintf` & etc functions (the VFS only supports 5 modes which can be found in `src/kernel/HAL/vfs.h`)
- Removed SSE instructions from kernel
- Turned on optimizations for kernel
- fixed IO operations
- Updated Makefile
