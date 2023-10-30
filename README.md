# WorldOS

## COPYING

Copyright (©) 2022-2023  Frosty515

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

## Latest Changes - 30/10/2023

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

## Resources used

- [OSDev Wiki](https://wiki.osdev.org/Main_Page) - used for documentation on almost everything
- [Intel x86_64 Software development manuals](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html) - used for instruction references and Page Tables
- [nanobyte_os](https://github.com/nanobyte-dev/nanobyte_os) - inspired the file layout, panic system, interrupt system and printf
- [TetrisOS by jdh](https://www.youtube.com/watch?v=FaILnmUYS_U) - inspired me to start this project. IDT code was helpful. rand function from this
- [Limine bootloader](https://github.com/limine-bootloader/limine) - bootloader being used
- [SerenityOS](https://github.com/SerenityOS/serenity) - inspired system call entry and file descriptor management

## Prerequisites

### Program Requirements

#### All

- bash

#### OS build/run

- mtools
- qemu version 4 or higher
- curl
- make
- git
- nasm
- ovmf

#### Toolchain build

- gcc/g++ version 9 or higher
- make
- bison
- flex
- gmp
- libmpc
- mpfr
- binutils version 2 or higher (only for binutils builds)
- libtool
- patch

#### mkgpt build

- gcc/g++ version 9 or higher
- m4
- autoconf
- automake 1.15 or newer
- binutils version 2 or higher

See notes if your system cannot meet these requirements

### System Requirements

- At least 256MiB able to be allocated to QEMU.
- Host that is capable of running x86_64 virtual machines in QEMU.
- Host must be capable of allocating 1 core to a x86_64 QEMU virtual machine.
- Host must have KVM support

If you cannot meet these requirements, see notes

### **Windows**

#### Windows 10 Method 1

- install WSL2 and install a distribution to it
- follow the steps for Linux -> (distribution you chose), except remove `qemu` from the install list (in WSL2)
- install `qemu` from [its website](https://www.qemu.org/download/#windows) and add it to `PATH`

#### Windows 10 Method 2

- install WSL2 and install a distribution to it
- follow the steps for Linux -> (distribution you chose)
- install an X-server on your Windows 10 host such as [VcXsrv](https://sourceforge.net/projects/vcxsrv/)
- in WSL2, run `export DISPLAY = :0` as non-root (I recommend adding this to your startup script in WSL2, otherwise you will have to run this every time you open WSL2)

#### Windows 11

- install WSLg and install a distribution to it
- follow the steps for Linux -> (the distribution you chose)

### Linux

#### Debian

- run `sudo apt update && sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo mtools curl qemu git m4 automake autoconf bash nasm libtool patch ovmf`

#### Fedora/RHEL

- run `sudo dnf install gcc gcc-c++ make bison flex gmp-devel libmpc-devel mpfr-devel texinfo mtools curl qemu git m4 automake autoconf binutils bash nasm libtool patch edk2-ovmf`

#### Arch

- run `sudo pacman -Syu base-devel gmp libmpc mpfr mtools curl qemu git bash nasm`

#### Gentoo

- run `sudo emerge --ask --verbose sys-devel/gcc sys-devel/make sys-devel/bison sys-devel/flex dev-libs/gmp dev-libs/mpc dev-libs/mpfr sys-apps/texinfo sys-fs/mtools net-misc/curl app-emulation/qemu dev-vcs/git sys-devel/m4 sys-devel/automake sys-devel/autoconf sys-devel/binutils apps-shells/bash dev-lang/nasm sys-devel/libtool sys-devel/patch sys-firmware/edk2-ovmf`

---

## Building

1. run `make -j<jobs>` in the appropriate place for your OS (WSL2 for Windows 10, WSLg for Windows 11, etc.). `<jobs>` should be the amount of parallel threads to run. NOTE: If the toolchain isn't built and installed to the correct location, it **will** be built and installed

## Build and Run - Unix like, Windows 11 and Windows 10 method 2

1. run `make -j<jobs> config=<config>` in the appropriate place for your OS (WSL2 for Windows 10, WSLg for Windows 11, etc.). `<jobs>` should be the amount of parallel threads to run. `<config>` should be either `debug` or `release`. NOTE: If the toolchain isn't built and installed to the correct location, it **will** be built and installed.

## Running - Other

Run the following command(s) in the appropriate place for your OS (WSL2 for Windows 10 Method 2, cmd/powershell for Windows 10 Method 1, etc.) based on what configuration you want.

### Debug

1. run `qemu-system-x86_64 -drive if=pflash,file=ovmf/x86-64/OVMF.fd,format=raw -drive format=raw,file=iso/hdimage.bin,index=0,media=disk -m 256M -debugcon stdio -machine accel=kvm -M q35 -cpu qemu64`

### Release

1. run `qemu-system-x86_64 -drive if=pflash,file=ovmf/x86-64/OVMF.fd,format=raw -drive format=raw,file=iso/hdimage.bin,index=0,media=disk -m 256M -machine accel=kvm -M q35 -cpu qemu64`

---

## Notes

### System Requirement info

- You might be able to run WorldOS with less RAM (by changing the '256M' to the amount of RAM you want), but I cannot guarantee that it would run on any less than 256MiB.
- You computer **must** be capable of virtualization to run WorldOS in a VM.
- You can allocate more than 1 core, but only 1 will be used.
- KVM can be disabled, you will just need to remove `-machine accel=kvm` from the QEMU command line. It doesn't make much of a performance boost by using it, so it doesn't *have* to be enabled.

### Program Requirement info

- You can use alternatives to `curl` (such as `wget`), but you will have to modify the `Makefile`.

### Building and Running

- Make sure to follow the setup steps before building and running.

### Other platforms

- All modern POSIX operating systems (including MacOS X) should be able to build and run this OS. It might just take a bit more effort.
- Windows versions that don't support WSL2 or WSLg might require a lot more effort to get this OS running, so I recommend to just install a Linux Virtual Machine on those.
