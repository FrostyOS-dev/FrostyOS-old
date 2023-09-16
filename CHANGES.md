# Changes

## Latest Changes - 16/09/2023

- Added `stdint.h` include to `utils/src/buildsymboltable.cpp` so it compiles on all platforms instead of just Gentoo Linux
- Started working on a very simple statically linked LibC. This includes the various crt*.o start and end files
- Moved system headers to LibC includes, instead of being in the root filesystem by default
- Implemented basic stack smashing protector support in the kernel
- Fix a spelling mistake in `Scheduler.cpp`
- Implemented RemoveThread and RemoveProcess functions in scheduler
- Added a scheduler resume function
- Changed the way the scheduler detects when there is nothing left to run so it will only check when it actually needs to run something
- Scheduler now panics when `g_current == nullptr` instead of just doing some weird assertion
- Updated include guards in `stdarg.h`
- Added a RemoveThread function to the process class
- Changed how system calls behave so they all go to one function, which then calls the appropriate function with the correct arguments
- Fixed VirtualPageManager and PageManager destructors so they perform proper cleanup
- Fixed ELF_Executable destructor and added an end_handler cleanup function
- Added proper argument and environment variable passing support to the ELF loader
- Added a Remap function to the PageManager class so permissions can be changed
- Added a thread cleanup function system
- Add support for main thread creation requesting to the Process class
- Added an extra argument to the `VirtualPageManager::LockPage(s)` functions that is used for optimisation
- Improved the `VirtualPageManager::UnfreePages` function for hopefully the last time
- Cleaned up various parts of the `PageManager` class
- Minor bug fixes to the `LinkedList::deleteNode` functions
- Changed `x86_64_kernel_switch` function so RFLAGS is set after RAX and RDI are set
- Implemented exit system call (Syscall 0)

## 30/08/2023

- Renamed `stdio.hpp` to `stdio.h` and marked all functions as `extern "C"`
- Added `memset`, `memcpy`, `memmove` and `memcmp` declarations to `string.h`
- Removed unnecessary includes from `kernel.hpp`
- Improved `VirtualPageManager::Unfree` function so it should now support all possible scenarios
- Added fixed address allocation to the VirtualPageManager and PageManager
- Added proper mapping permission support to PageManager with user support
- Added region expansion support to the VirtualPageManager and PageManager
- Added region automatic expansion support for user PageManagers
- Added multiple privilege level support to the Scheduler with the standard kernel priority and 3 different user priorities: LOW, NORMAL, HIGH. Each privilege level gets 80% more CPU time than the level below it. If there are no tasks at a certain level, the level below gets that rotation
- Added basic system call support. Currently all 64 system calls just redirect to the same function which just prints the number and the arguments. The system call number is placed in `rax` and the arguments are parsed in `rdi`, `rsi` and `rdx`, return value is in `rax` (just like the System V x86_64 calling convention).
- Added virtual region allocation support to the Process class. Currently, 16MiB is allocated, but this is not the preferred method for allocation
- Added basic ELF executable loading and execution. Currently, relocation is not support.
- Added a `LinkedList::deleteNode` function that can delete a specific node, instead of finding it based off its data.
- Removed unused debugging argument from `x86_64_LoadCR3` function.
- Updated `x86_64_InitPaging` function so it reserves all of non-canonical address space instead of a small section.

## 26/08/2023

- Fixed `VirtualPageManager::UnfreePages` function so it now can unfree pages with page counts that don't directly match a size
- General VirtualPageManager cleanup
- Switched PageManager to use a virtual region based system with a built in VirtualPageManager, making PageManager class more generic
- Added an `InitVPageMgr` to the VirtualPageManager class for region-only initialisation
- Changed `x86_64_InitPaging` to use the new initialisation function for the VPM

## 25/08/2023

- Changed `__assert_failed` to use new panic system
- Changed PagingInit, PageManager and PhysicalPageFrameAllocator to use new panic system
- Changed Scheduler to use new panic system
- Changed `StartKernel` to use new panic system

## 19/08/2023

- Implemented support for panic register saving outside of interrupts/exceptions with a `x86_64_PrePanic` function
- Added a `PANIC(reason)` macro for easier panic outside of interrupts/exceptions
- Added a check inside `x86_64_Panic` to ensure we don't get a page fault due to a null panic reason

## 18/08/2023 (Afternoon)

- Removed unnecessary debug printing from `VirtualRegion` class
- Added a global instance of the `VirtualPageManager` class for all address space
- Fixed paging initialisation so ACPI memory map entries are not identity mapped

## 18/08/2023

- Added extra pixel information to the `Framebuffer` struct
- Added a colour format class for better support with bits per pixel other than 32
- Changed the `BasicVGA` class to use the `Colour` class with the new colour format support
- Added support for 8, 16, 24, 40, 48, 56, 64 bits per pixel

## 16/08/2023

- Fixed ACPI tables code so all addresses are converted to HHDM addresses
- Fixed `HAL_Stage2` so the PCI buses address is converted to a HHDM address
- Changed Paging initialisation code so that the first 4GiB is only mapped to HDDM address space and not identity mapped
- Changed `x86_64_Panic` so a stack trace is only performed for interrupts/exceptions
- Fixed `FileStream::isOpen` so the correct return type is returned when the internal inode is either null or not a file
- Added a `GetSize` function to the `TempFSInode` class
- Added a `GetSize` function to file streams
- Reordered user GDT segments so they will work better with system calls later

## 15/08/2023

- Added a utils folder
- Added a `buildsymboltable` utility to convert the kernel symbol table generated by `x86_64-worldos-nm` to an easier format for the kernel to use
- Added basic stack tracing to `panic`
- Fixed `USTAR_Lookup` so it now works correctly
- Fixed a write-strings warning in `Initialise_InitRAMFS`
- Added a proper symbol resolving system

## 12/08/2023

- Added some basic C header files to `root/data/include/`
- Added basic Symbolic link support to TempFS
- Added intra-mount-point Symbolic link support to the VFS
- Added `toolchain` directory to the gitignore
- Added custom `x86_64-worldos` toolchain support
- Cleaned up `README.md`

## 08/08/2023

- Fixed `fwrite` function so it now actually works
- Added `GetParent` function to `Inode` base class and applied changes accordingly to `TempFS::TempFSInode` class
- Fixed `VFS::CreateFolder` function so it removes a trailing slash from the name if present.
- Fixed `VFS::GetMountpoint` function so it works with subdirectories within the root filesystem
- Fixed `VFS::OpenStream` function so it actually adds the newly created stream to the `VFS` class's internal data structures
- Implemented `VFS::CloseStream` function
- Added `Kernel_Stage2Params` structure so multiple arguments can be passed to the kernel Stage2 instead of the old one argument
- Added basic tar initial RAMFS support via limine kernel modules. All root filesystem files/folders are placed under `root/`
- Added `example.txt` file under `root/`

## 06/08/2023

- Added count check to `PageManager::AllocatePages` so `PageManager::AllocatePage` is called when `count` is 1
- Implemented simple `rand` and `srand` functions based on implementation from TetrisOS by jdh
- Added `PAGE_SIZE` macro to `util.h`
- Removed unnecessary include from `stdlib.c`
- Implemented simple temp (RAM) filesystem
- Implemented simple VFS. Currently only fully supports root mountpoint
- Implemented simple file stream.

## 31/07/2023

- Added `krealloc` function
- Added declarations for `kcalloc`, `kfree`, `kmalloc` and `krealloc` to `stdlib.h` so we are ISO C compliant
- Made `kcalloc` ISO C compliant
- Cleaned up `stdlib.h`
- Fixed up header guards in `stdbool.h`, `stddef.h`, `stdio.hpp`, `stdlib.h`, `string.h` and `time.h`
- Added Copyright header to all C/C++ header and source files, all x86 assembly source files, kernel linker script, all `Makefile`s

## 25/07/2023

- Added CLion files to .gitignore
- Added parallel build support to `Makefile` and `kernel/Makefile`
- Moved run code into `Makefile`
- Added proper build/run configuration support to `Makefile`

## 18/07/2023

- Created simple colour class
- Refactored simple VGA code into a `BasicVGA` class
- Add simple TTY class
- Added double buffering support
- Added a x86_64 panic VGA device
- Added a scheduler `Stop()` function
- Fixed struct definitions for VGA Framebuffer and Position

## 09/07/2023

- Added system segment support to GDT
- Implemented simple TSS

## 08/07/2023

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
