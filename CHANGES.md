# Changes

## Latest Changes - 29/04/2024

- Added `ubsan` support.
- The bitmap in the PhysicalPageFrameAllocator is now cleared to all 1s before usage to ensure all pages default to being used/reserved.
- Added ubsan disabled attributes to page mapping functions, ELF Symbol table init and ACPI `getOtherSDT` to prevent false positives.
- Updated image size to be 10MiB so larger builds of the kernel with ubsan enabled can fit.
- Fixed a spelling mistake in change list for previous commit.

## 08/04/2024

- Update the initramfs initialisation to check if the file size of an item is zero.
- Convert build system over to CMake. The old Makefile-based system will remain for now, but it is deprecated.
- The toolchain and mkgpt are now built with shell scripts called from CMake.
- OVMF_VARS is now updated using a shell script called from CMake.
- Updated resources that are used.
- Updated build/run instructions.
- Updated various requirements to be less strict as it is unnecessary.

## 21/02/2024

- Fixed the toolchain build steps so it actually builds the toolchain.
- Updated the `printf` family of functions to now support all flags and precision specifiers in the kernel and LibC. Still no floating point support yet.
- Implemented the `sprintf` family of functions in the kernel and LibC. They have the same functionality as the `printf` family of functions.
- Fixed `PageFaultHandler` to use the `snprintf` function to format the error message instead of an inlined weird kernel panic.
- Implemented basic spinlock support. They currently aren't used anywhere.

## 15/02/2024

- Updated `PageFaultHandler` to actually display panic output instead of a simple page fault description.
- Updated paging initialisation code to use 2MiB pages where possible when mapping memory map entries after 4GiB and actually check if it needs to be mapped or not properly.
- Updated `UpdateMemorySize` function to include entry type `KERNEL_AND_MODULES` as a valid entry to actually map and include in the memory size.
- Updated kernel heap to set the `next` parameter of the first block to `nullptr` when initialising the heap.

## 20/01/2024

- kmalloc improvements.
- Userland thread exit improvements to prevent scheduling issues.
- Fixed `open` syscall and kernel lib function `internal_open` when creating a file.
- Added a separate *head* system for TempFSInodes. This means that an Inode's stream state can be different for each file stream that might have it open.
- Updated VFS to create the name for an Inode instead of just using the name provided to it as the name for the inode. This was a massive security flaw.
- Moved `util.c` and `util.h` into kernel lib.
- Implemented proper file system destruction.
- Implemented multiple mount point support. This required a complete rewrite of the path resolution and file/folder/symlink creation code. Symlinks can still only point to something in the same mount point.
- Implemented `mount` and `unmount` syscalls.
- Various other minor code clean-up across the VFS.

## 12/01/2024

- Updated to limine 6.x.
- Updated to limine protocol revision 1.
- Fixed optimised memset function so it actually works.
- Implemented LibC `ctype.h` functions.
- Added ISO C compliance note to LibC's `stdio.h` about `gets`.

## 02/01/2024

- Implemented `IN_BOUNDS` macro which just determines if a value is inside of an inclusive range.
- Added a check to `Scheduler::GetCurrent` to ensure we only return the current thread if the scheduler is running.
- Added proper page fault handling support. This comes with a special message to say exactly what happened.
- Added `PageManager::GetPermissions` functions to get the exact permissions of a mapped page.
- Added less than, greater than, less or equal and greater or equal functions to the Priority enum, to allow for better determining if a process has a higher or low priority than another.
- Implemented basic signalling support. This means a process can send a signal to another process with a priority less than or equal to it's priority. These signals can be caught by the process and handled. Signals are **always** handled on the main thread of a process.
- Implemented `onsignal`, `sendsig` and `sigreturn` syscalls. When a signal handler registered by `onsignal` is called, some code injection occurs so that the `sigreturn` syscall is called to perform clean-up after the signal handler and restore the thread state.
- Implemented thread-specific exception handling. When an exception occurs in a process, the main thread is sent an appropriate signal. On x86_64, divide by zero and floating point exceptions send a `SIGFPE` signal, Invalid Opcode sends a `SIGILL` signal, and page faults and general protection faults send a `SIGSEGV` signal.
- Implemented `signal` and `raise` LibC functions. These are just wrappers around the `sendsig` and `onsignal` syscalls.
- Updated `abort` in LibC to `raise` a `SIGABRT` signal, instead of just exiting with status `EXIT_FAILURE`.

## 29/12/2023

- Updated `pid_t` and `tid_t` types to be `long int` instead of `unsigned long int` to allow for better error reporting.
- Updated `exec` system call to return the PID of the new process instead of 0.
- Renamed `Programs` to `Utilities` as it is a more fitting name.
- Added `SYSROOT` variable to the root Makefile to allow for easier changing of the sysroot. This is used by all sub-Makefiles.
- Various variables are now exported in the root Makefile to allow for easier usage of them in sub-Makefiles. These variables include, but are not limited to: `CC`, `CXX`, `ASM` and `config`.
- Updated sub-Makefiles to not include the parent Makefile as it is not needed.
- Added a proper userland folder instead of some userland things scattered in the root directory of the project. LibC is now at `Userland/Libraries/LibC` and the Utilities folder is at `Userland/Programs/Utilities`. Each directory level has its own `Makefile` to make building easier.
- Updated resources.
- Removed `patches` folder as it is no longer needed.

## 24/12/2023

- Updated `buildsymboltable` to just take the raw output from `nm` into stdin instead of trying to execute it and create pipes.
- Updated `ls` to print the error message from `stat` over stderr instead of using perror since the errno value is not set on raw syscalls.
- Implemented `GetSubInode` function in TempFS, which is almost the same as `GetInode` except the starting inode is already determined.
- Created a global `GetChild` function in inodes.
- Removed some unnecessary debug messages in `kernel/src/fs/FileDescriptor.cpp`.
- Implemented public `GetMountpoint` in the VFS which gets the mountpoint for an FS.
- Implemented `Process::IsMainThread` function.
- Implemented working directory support. All threads have their own working directory, and all processes have a default working directory which is inherited from the parent process, and can be changed with by the main thread. This is currently unsupported on file, folder and symlink creation.
- Implemented `chdir` and `fchdir` syscalls.

## 22/12/2023

- Update toolchain path in all the run-utils scripts.
- Added git to toolchain build requirements.
- Updated run command in README.md.
- Updated `stat` program to print the file type.
- Created a new custom kernel heap. Currently this does not support self expansion and shrinking due to circular dependencies.
- Added a userspace heap allocator to LibC based off the kernel heap, except is supports self expansion and shrinking. Currently, it does not support `realloc` at all.
- Implemented `_Exit` function in LibC.

## 21/12/2023

- Updated `Buffer::ClearUntil` function to return the number of blocks deleted.
- Updated `Buffer::AddBlock` function to return a pointer to the block added.
- Updated `Buffer::Write` function to keep track of the current block in a different way so simultaneous reads and writes can be done better.
- Updated `KeyboardInput` class to use separate read and write offsets, so simultaneous reads and writes can be done better.
- Add a directory stream class which enumerates through inodes in a directory. This support was added to file descriptors.
- Added some global `Inode` functions which used to be only exclusive to `TempFSInode`s.
- Added some global `FileSystem` functions which used to be only exclusive to `TempFileSystem`s.
- Updated `stat` system call to return the type of the file.
- Added `getdirents` system call to get a certain number of directory entries in the position in the stream.
- Updated `open` system call to allow opening directories.
- Added extra include to `kernel/lib/stdio.cpp` as it should already has exited.
- Updated `buildsymboltable` util to use the new toolchain directory.
- Implemented basic `ls` program.
- Updated `stat` program to pre-initialise the `stat_buf` structure correctly.

## 19/12/2023 Afternoon

- Moved toolchain prefix to `toolchain/local` to avoid conflicts with other toolchains.
- Updated GCC to 13.2.1
- Added additional parameter to `seek` system call to allow seeking from a pre-defined position instead of always seeking from the start of the file. This pre-defined position can be either the start of the file, the current position or the end of the file.
- Updated LibC `fseek` to utilise new `seek` support.
- Updated LibC `stdio.h` header to only set the `SEEK_SET`, `SEEK_CUR` and `SEEK_END` macros if they aren't already defined.

## 19/12/2023

- Add `ENOLCK` errno code, which is identical to `ENOLOCK`.
- Implemented ISO C11 compliant `locale.h` and `setjmp.h` header files.
- Add `fdopen` function declaration to `stdio.h`.
- Updated toolchain build script to use git repositories instead of tarballs. This included telling the binutils configure script explicitly to not build gdb.
- Moved kernel headers to `kernel/headers` instead of `kernel/include`.
- Added a kernel library to `kernel/lib`, which just as all the kernel variants of the LibC code. This used to be part of the main kernel code, but it was moved to make the kernel code cleaner.
- Added some C++ headers which wrap the C headers to the kernel.

## 08/12/2023

- Fixed x86_64 system call entry function. Some spelling mistakes, segment errors and improper saving of user stack were fixed.
- Added optional process priority parameter to `Execute` and `ELF_Executable::Execute` functions. If not specified, the priority will be set to NORMAL.
- Implemented sleeping thread support in the scheduler. Threads must for a time in multiples of 5ms currently has the timer is running at 200Hz.
- Add `sleep` system call which sleeps the current thread for the specified amount of time in seconds.
- Add `msleep` system call which sleeps the current thread for the specified amount of time in milliseconds.
- Added useful timer macros to `HAL/time.h` to make creating timers easier.
- Added `MS_PER_SCHEDULER_CYCLE` macro which gives the number of milliseconds for each scheduler cycle.

## 07/12/2023

- Either fixed or created `ctype.h`, `fenv.h`, `inttypes.h`, `math.h`, `signal.h`, `stdio.h`, `stdlib.h`, `string.h` and `time.h` ISO C headers to make them compliant with the C99 standard. Most functions have not been implemented. Some that already existed have been fixed, and a couple of simple ones have been implemented that are similar to already implemented ones.
- Updated `stat`, `chown` and `chmod` to use the new `perror` function.
- Updated `cat` and `echo` to use correct `stdio.h` functions.

## 06/12/2023 Evening

- Implemented proper ELF error reporting.
- Updated write system call to just return success if the size is 0.
- Implemented `Execute` function in the kernel which loads an ELF file and executes it.
- Implemented `exec` system call, which is a wrapper around `Execute`.
- Implemented `is_leap_year` function that properly checks if a year is a leap year. This is now used in functions `years_to_days_since_epoch` and `days_since_epoch`
- Add spacing to `GetWeekDay` function in RTC driver to make it more readable.
- Updated `stdbool` include in kernel x86_64 syscall header to always be included, as the header has C++ checks.

## 06/12/2023

- Added `stdbool` include to kernel x86_64 syscall header when we are in C.
- Implemented PID and TID support. Each process has a unique 64-bit PID, and within each process, each thread has a unique 64-bit TID.
- Implemented `getpid` and `gettid` syscalls.

## 02/12/2023

- Implemented proper `strtol` and `strtoul` functions in LibC. These currently do not support detecting the base.
- Implemented `VFS::GetInode` function which optionally outputs the file system of the inode.
- Implemented `FileSystem::GetType` function.
- Implemented `FileStream::GetInode` and `FileStream::GetFileSystem` functions.
- Implemented proper file permissions in TempFS, the VFS, initramfs, file streams, file descriptors and processes. Processes store UID, GID, effective UID and effective GID.
- Implemented `getuid`, `getgid`, `geteuid` and `getegid` system calls.
- Implemented `chown` and `fchown` system calls. These are currently non-recursive.
- Implemented `chmod` and `fchmod` system calls. These are currently non-recursive.
- Implemented `stat` and `fstat` system calls. These currently only support files. This returns the UID, GID, mode and size of a file.
- Updated `open` system call to take a mode argument which determines the mode (ACL) for a file if it is created.
- Created simple `chown` command.
- Created simple `chmod` command.
- Created simple `stat` command.
- Updated initramfs creation in `Makefile` to set the UID and GID of all files to 0.
- Added `GNU tar` to build dependencies.

## 30/11/2023

- Implemented Partial read/write support. This includes counting the number characters printed to debug and the display.
- Implemented EOF support.
- Implemented `getc` and `fgetc` support in kernel and LibC.
- Implemented Eternal support in the `SimpleLinkedList` data structure.
- Updated ELF symbol table class to take advantage of eternal support in `SimpleLinkedList`.
- Updated KeyboardInput to use a heap allocated `Buffer` class instead of internal `Buffer` to prevent against some use-after-free issues.
- Slight reordering of `Buffer::DeleteBlock` function to prevent against use-after-free issues.
- Add a `IsValidChar` function to check if a character is displayable.
- Updated `BasicVGA::putc` function to use the `IsValidChar` function instead of just printing a space for un-displayable characters.
- Disabled printing of debug message when PS/2 keyboard is initialized.
- Implemented better error handling in `Thread::sys$write` and `internal_write` functions.
- Implemented proper error handling in `internal_read` function.
- Minor changes to `isr_common` function on x86_64 to help with proper stack trace generation and registers and now passed as a pointer in `RDI` instead of raw data on the stack.
- Added page size check to `x86_64_unmap_page` and fixed a bug where the wrong page level was being set as not present.
- Added EOF check to `KeyboardInput::GetChar` function.
- Updated `TTY::getc` to loop until a valid character is available.
- Implemented x86_64 specific faster `memcpy` and `memset` functions in the kernel which use 64-bit operations for most of the buffer, then fall back to 8-bit operations for the rest. All usages of the old `fast_memcpy` and `fast_memset` functions should eventually be updated to use this new implementation.
- Updated `crt0.asm` to push and pop important caller-saved registers after call to `__init_libc` function.
- Implemented basic programs directory. These programs only have one source file each, and once compiled, get copied to the `/data/bin` directory in the system root. A VSCode C/C++ configuration was also added in replacement for the old `Test-program` configuration.
- Implemented basic `cat` program.
- Implemented basic `echo` program.

## 08/11/2023

- Fixed various `BasicVGA` functions such as `putc`, `NewLine` and `ScrollText`
- Added total thread counter to scheduler. This allows for faster task switching if there are no other threads to switch to.
- Cleaned up code style in `TTY.cpp`
- Implemented basic stack data structure. Currently it just holds a stack of pointers.
- Implemented basic dynamic buffer data structure. It holds a doubly-linked list of blocks. This prevents reallocation every time the buffer needs to increase in size.
- Implemented basic Intel 8042 PS/2 controller driver.
- Implemented basic PS/2 keyboard driver.
- Added keyboard input support to TTYs. This also makes stdin function instead of just outputting zeros.
- Added `-Wno-switch` flag when building kernel to prevent massive blocks of useless warnings about the PS/2 keyboard scancode handling.
- Fixed the maximum value for `uint8_t` in the kernel's `stdint.h`

## 04/11/2023

- Fixed bit masking in `x86_64_unmap_page` and `x86_64_remap_large_page`.
- Removed call to `check` function at the end of `mrvn_free`.
- Removed unnecessary debug printing in kernel `strrchr`, a `SimpleLinkedList` function and during ELF loading and user page table initialisation.
- Updated the Scheduler's `PrintThreads` function to only print thread level name if there are threads at that level.
- Implemented `x86_64_PrepareThreadExit` function which switches to the global stack and then calls the actual exit function.
- Implemented new `do_exit` function which actually destroys the thread. The old function calls the x86_64 helper with the arguments required to call the real function after stopping the scheduler if necessary.
- Implemented simple kernel headers with simple inline-able wrappers around system calls. These get copied over to the system root prior to LibC building
- Added define to kernel parameters so the kernel headers can be included in the kernel.
- Updated LibC to use the system call wrappers in the kernel headers instead of it's weird headers and slow wrapper functions. This also makes LibSSP fully independent of LibC.
- Fixed kernel's `SystemCall.hpp` to use the new kernel headers instead of defining the system call numbers separately.
- LibSSP is now copied to the system root as `libssp_nonshared.a` and `libssp.a`. This could probably be done with a symlink, but this is only temporary.

## 30/10/2023

- Added read/write size returning to `fread` and `fwrite` in the kernel.
- Fixed `TempFSInode::Seek` so seek is performed correctly.
- Added kernel stack size macros to kernel `util.h`.
- Updated system call entry to get the current thread prior to calling the actual system call handler.
- Now disable interrupts while getting the current thread in the system call entry.
- Implemented better enter_user function that uses `iretq` instead of `sysretq`.
- Updated scheduler resume function to immediately switch to the new thread instead of waiting for the next scheduler tick.
- Set the stack segment to the data segment in the `x86_64_PrepareNewRegisters` function.
- Interrupt flag now gets masked by the CPU on system call entry.
- Removed the `x86_64_WorldOS` namespace that was being used in `ELFKernel.hpp` and `ELFKernel.cpp`.
- Updated x86_64 IO header guards.
- Implemented `Thread::PrintInfo` function
- Implemented `Scheduler:PrintThreads` function
- Now print the scheduler threads to debug in Panic.
- Changed the existing `KERNEL_STACK_SIZE` macro to `INITIAL_KERNEL_STACK_SIZE`.
- Added a new `KERNEL_STACK_SIZE` macro with defines the kernel stack size to be used after init. It is set to 64KiB.
- Updated all page mapping related functions to take the Level4Group as an argument.
- Implemented PageTable class which handles all mappings, unmappings, permission updates, physical address retrieval and page permissions decoding.
- Removed all the old usages of the old mapping, unmapping, remapping and physical address retrieval functions.
- Implemented basic address space separation. Each process gets its own address space. This is mostly setup in the ELF loader currently. The kernel and HHDM is mapped as shared memory in all address spaces.
- The CR3 value is changed to the kernel value at the start of the exit system call as that address space is about to be destroyed.
- Updated `x86_64_unmap_page` and the large page equivalent to free the unused page tables.
- Fixed `void Scheduler::Next(void* iregs)` to actually use the parsed argument and not create some weird x86_64_Interrupt_Registers local variable. This has been a bug since userland support was first added. It is only now been finally discovered.
- Each thread now has its own kernel stack. This is important because the thread can now be interrupt during kernel functions and still behave correctly.
- Implemented a `PrintRegions` function to the PageManager class.
- Updated memory size detection to use the end address of the last free memory map entry as the end of memory. This prevented some weird behaviour in QEMU when KVM is off, where massive reserved sections of memory would be detected after the real end of memory.
- General cleanup of the `x86_64_InitPaging` function.

## 15/10/2023 (Afternoon)

- Optimised physical memory allocation using new `m_nextFree` internal variable.
- Implemented out of memory checking in physical memory manager
- Implemented x86_64 remapping functions for standard sized pages and large pages
- Implemented proper page remapping support to `PageManager` class instead of un-mapping, then mapping again
- Removed `physical_address` member from `PageObject` struct
- Updated `PageManager` class to allocate 1 physical page at a time on demand
- Removed ovmf firmware from repository and use host system's instead. Variables are copied and accessed as read-write, code is loaded in-place as read-only.
- Updated `clean-all` make target to remove local ovmf directory

## 15/10/2023

- Implemented eternal heap using a basic bump allocator.
- Added eternal symbol table loading support to `ELFSymbols` class.
- Implemented `ReservePage(s)` functions to `PageManager` to allow for reserve virtual address space without actually allocating physical memory.
- Added support to `AllocatePage(s)` functions to allow for allocating physical memory and mapping it to previously reserved virtual memory.

## 13/10/2023 (Evening)

- Implemented logarithmic base-2 function in the kernel for unsigned integers (currently just rounds down the result)
- Updated ELF loader to use new function

## 13/10/2023

- Updated the `printf` family of functions in kernel and LibC to support force_sign and zero_pad flags, width specifier, keeping track of characters printed, uppercase printing
- Improved panic screen layout
- Updated panic screen to use new printf functionality
- Updated ISR handler to use new printf functionality
- Updated stack tracing to use new printf functionality

## 05/10/2023

- Added address alignment and division macros to kernel util header
- Setting upper 12-bits of page mapping flags actually works. This means that no-execute protection is actually active
- Only set NX bit for lowest page table level for the relevant page size
- Updating kernel mapping to use noflush variant of the `x86_64_map_page` function to avoid unnecessary TLB flushing
- Added `isValidAllocation` function to PageManager
- Removed unnecessary debug print statement from ELF loader
- Implemented mmap, mprotect and munmap system calls. Region expansion for mmap with an address request is not supported. Splitting of allocations is unsupported in mprotect and munmap.
- Moved LibC stack_protector to separate library due to GCC requirements
- Updated GCC patch so libgcc actually builds
- Updated Makefile so gcc version check stderr is discarded
- Removed old libgcc patch

## 24/09/2023

- Changed syscall entry so interrupts can be enabled while in system calls. This also means that the Kernel GS base is only accessible when needed.
- No longer set kernel gs base in `x86_64_enter_user` because it is redundant as the scheduler already does this.
- Added alignment member to the end of the `x86_64_Registers` structure
- Implemented `div`, `udiv`, `ldiv` and `uldiv` functions in assembly in kernel instead of in C due to inconsistencies in the compiler
- Improved bitmap class so the code is nicer and more efficient
- Changed RegisterFrame layout in Thread class to allow user stack to be saved easier and for the kernel stack to be accessed easier
- Added some read/write validation functions to the process class so userspace pointers/strings can be checked before being used by the kernel
- Added a SyncRegion function to the Process class to ensure the VirtualRegion of the Process matches the VirtualRegion of the Process's PageManager
- Added an `IsValidPath` function to the VFS to check if a path exists or not
- Marked the `VFS::GetMountpoint` function has `const` so it can be called in more places
- Copied `strchr` and `strrchr` functions from LibC over to the kernel
- Added all the C++11 errno values to `LibC/include/errno.h` and copied them over to the kernel
- Added read function to TTY that simply zeros the buffer provided as no user input mechanism is support
- Added proper file descriptor management. A file descriptor is a non-zero 64-bit signed integer that represents a TTY, a FileStream or Debug
- Converted all the kernel stdio functions to use this new system and implemented `fopen`, `fclose`, `fread` and `fseek` functions.
- Added a FileDescriptorManager to the Thread class. By default stdin is opened as read-only with ID 0 to KTTY, stdout and stderr as write-only with ID 1 and 2, respectively, to KTTY and stddebug as write-only with ID 3 to Debug
- Implemented `open`, `close`, `read`, `write` and `seek` system calls
- Copied over kernel stdio code to LibC with minor modifications. Currently only 8 streams can be opened simultaneously, not including stdin, stdout, stderr and stddebug. No streams are buffered as userland memory allocation is not supported yet.
- Implemented basic C assertions in LibC. Currently only prints to debug, but can print to stdout if a line is uncommented.
- Implemented stack protector in LibC

## 16/09/2023

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
