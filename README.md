# FrostyOS (formerly WorldOS)

## NOTE

This is unstable code and does not boot successfully.

## COPYING

Copyright (©) 2022-2024  Frosty515

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

## Latest Changes - 12/05/2024

- Implemented LAPIC support (including multicore start-up support)
- Implemented I/O APIC support. This changed the whole IRQ system.
- Implemented HPET support. This is now the main kernel timer.
- Updated the scheduler to utilise multiple cores.
- Switched scheduler over to using the HPET for timing.
- Scheduler now used intrusive thread lists to prevent heap allocation on task switching.
- Implemented idle threads. These threads always run in kernel mode and are used to keep the CPU busy when no other threads are running.
- Converted all of the VFS to use errno values instead of error enums.
- Implemented spinlocks in the VFS, PMM, VMM, PM, scheduler, and TTYs.
- Implemented `invlpg` instruction in the VMM. This is used where possible instead of reloading the whole page table.
- Changed system call names in the kernel to not use the $ sign in there names.
- Add pragmas for GCC and Clang to silence certain warnings which are valid.
- Implemented IPI support. This is used to send inter-processor interrupts to other cores.
- On panic, a stop IPI is sent to all other cores to stop them from running.
- Implemented TLB shootdowns. This is used to invalidate the TLB on all cores when a page table is changed.
- Added IPI for the scheduler to send to other cores to tell them to reschedule.
- Implemented semaphores and mutexes in the kernel. This required thread blocking in the scheduler. There are also some new system calls for these.
- Various other bug fixes and improvements to the kernel.
- Updated the README to mention multicore support.
- Updated QEMU run command to run with 2 cores by default.

## Resources used

- [OSDev Wiki](https://wiki.osdev.org/Main_Page) - used for documentation on almost everything
- [Intel x86_64 Software development manuals](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html) - used for instruction references and Page Tables
- [nanobyte_os](https://github.com/nanobyte-dev/nanobyte_os) - inspired the file layout, panic system, interrupt system and printf
- [TetrisOS by jdh](https://www.youtube.com/watch?v=FaILnmUYS_U) - inspired me to start this project. IDT code was helpful. `rand` function from this project
- [Limine bootloader](https://github.com/limine-bootloader/limine) - bootloader being used
- [SerenityOS](https://github.com/SerenityOS/serenity) - inspired system call entry, file descriptor management, directory layout and build system.

## Prerequisites

### Program Requirements

#### All

- bash

#### OS build/run

- mtools
- recent QEMU
- curl
- make (only for using legacy build system)
- git
- nasm
- ovmf (assumed to be installed to /usr/share/edk2/x64)
- POSIX-compatible tar
- CMake
- Ninja

#### Toolchain build

- recent gcc/g++
- make
- bison
- flex
- gmp
- libmpc
- mpfr
- recent binutils
- libtool
- git

#### mkgpt build

- recent gcc/g++
- recent binutils
- make

See notes if your system cannot meet these requirements

### System Requirements

- The default is for 256MiB to be allocated to QEMU, but all this isn't really needed.
- Host that is capable of running x86_64 virtual machines in QEMU.
- Default is to use KVM acceleration, but it can be disabled.

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

- run `sudo apt update && sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo mtools curl qemu git bash nasm libtool patch ovmf cmake ninja-build`

#### Fedora/RHEL

- run `sudo dnf install gcc gcc-c++ make bison flex gmp-devel libmpc-devel mpfr-devel texinfo mtools curl qemu gitbinutils bash nasm libtool patch edk2-ovmf cmake ninja-build`

#### Arch

- run `sudo pacman -Syu base-devel gmp libmpc mpfr mtools curl qemu git bash nasm cmake ninja`

#### Gentoo

- run `sudo emerge --ask --verbose sys-devel/gcc sys-devel/make sys-devel/bison sys-devel/flex dev-libs/gmp dev-libs/mpc dev-libs/mpfr sys-apps/texinfo sys-fs/mtools net-misc/curl app-emulation/qemu dev-vcs/git sys-devel/binutils apps-shells/bash dev-lang/nasm sys-devel/libtool sys-devel/patch sys-firmware/edk2-ovmf dev-build/cmake dev-build/ninja`

---

## Building

1. run `cd utils && mkdir build && cd build && cmake -GNinja .. && ninja && ninja install && cd .. && rm -fr build && cd ..` in the appropriate place for your OS (WSL2 for Windows 10, WSLg for Windows 11, etc.). This will automatically build with the number of jobs that `nproc` reports are possible. This will build and install the host system utilities to `utils/bin`. **This only needs to be run for the first build.**
2. run `build-scripts/enter-environment.sh`. This will drop you into a new shell with your `$PATH` updated to include the toolchain. **This only needs to be run once per terminal session.**
3. run `FROSTYOS_BUILD_CONFIG=<config> build-scripts/build.sh` in the appropriate place for your OS (WSL2 for Windows 10, WSLg for Windows 11, etc.). `<config>` should be either `Debug` or `Release`. This will automatically build with the number of jobs that `nproc` reports are possible. NOTE: If the toolchain isn't built and installed to the correct location, it **will** be built and installed.

## Build and Run - Unix like, Windows 11 and Windows 10 method 2

1. run `cd utils && mkdir build && cd build && cmake -GNinja .. && ninja && ninja install && cd .. && rm -fr build && cd ..` in the appropriate place for your OS (WSL2 for Windows 10, WSLg for Windows 11, etc.). This will automatically build with the number of jobs that `nproc` reports are possible. This will build and install the host system utilities to `utils/bin`. **This only needs to be run for the first build.**
2. run `build-scripts/enter-environment.sh`. This will drop you into a new shell with your `$PATH` updated to include the toolchain. **This only needs to be run once per terminal session.**
3. run `FROSTYOS_BUILD_CONFIG=<config> build-scripts/run.sh` in the appropriate place for your OS (WSL2 for Windows 10, WSLg for Windows 11, etc.). `<config>` should be either `Debug` or `Release`. This will automatically build with the number of jobs that `nproc` reports are possible. NOTE: If the toolchain isn't built and installed to the correct location, it **will** be built and installed.

## Running - Other

Run the following command(s) in the appropriate place for your OS (WSL2 for Windows 10 Method 2, cmd/powershell for Windows 10 Method 1, etc.) based on what configuration you want.

### Debug

1. run `qemu-system-x86_64 -drive if=pflash,file=/usr/share/edk2/x64/OVMF_CODE.fd,format=raw,readonly=on -drive if=pflash,file=ovmf/x86-64/OVMF_VARS.fd,format=raw -drive format=raw,file=iso/hdimage.bin,index=0,media=disk -m 256M -debugcon stdio -machine accel=kvm -M q35 -cpu qemu64 -smp 2`

### Release

1. run `qemu-system-x86_64 -drive if=pflash,file=/usr/share/edk2/x64/OVMF_CODE.fd,format=raw,readonly=on -drive if=pflash,file=ovmf/x86-64/OVMF_VARS.fd,format=raw -drive format=raw,file=iso/hdimage.bin,index=0,media=disk -m 256M -machine accel=kvm -M q35 -cpu qemu64 -smp 2`

---

## Notes

### System Requirement info

- You should be able to run FrostyOS with less RAM (by changing the '256M' to the amount of RAM you want), but I have not test it with less.
- There is no point trying to allocate more than 1 core, as only the first core is used.
- KVM can be disabled, you will just need to remove `-machine accel=kvm` from the QEMU command line. It doesn't make much of a performance boost by using it, so it doesn't *have* to be enabled.

### Program Requirement info

- You can use alternatives to `curl` (such as `wget`), but you will have to modify the `build-scripts/utils.cmake`.

### Building and Running

- Make sure to follow the setup steps before building and running.
- By default the OS runs with 2 cores. This can be changed by modifying the `qemu-system-x86_64` command line.

### Other platforms

- All modern POSIX operating systems (including MacOS X) should be able to build and run this OS. It might just take a bit more effort.
- Windows versions that don't support WSL2 or WSLg might require a lot more effort to get this OS running, so I recommend to just install a Linux Virtual Machine on those.
