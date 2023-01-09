# WorldOS

## Latest Changes - 09/01/2023

- Added Node pool for AVL Tree in case MM isn't active
- Added utility for new and delete
- Improved new and delete so they can check what they should do
- renamed `newdelete.h` to `newdelete.hpp`
- make toolchain check for nasm correctly

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

#### Toolchain and nasm build

- gcc/g++ version 9 or higher
- make
- bison
- flex
- gmp
- libmpc
- mpfr
- binutils version 2 or higher (only for binutils and nasm builds)

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

If you cannot meet these requirements, see notes

### **Windows**

#### Windows 10 Method 1

- install WSL2 and install a distribution to it
- follow the steps for Linux -> (distribution you chose), except remove `qemu` from the install list (in WSL2)
- install `qemu` from its website and add it to `PATH`

#### Windows 10 Method 2

- install WSL2 and install a distribution to it
- follow the steps for Linux -> (distribution you chose)
- install an X-server on your Windows 10 host such as [VcXsrv](https://sourceforge.net/projects/vcxsrv/)
- in WSL2, run `export DISPLAY = :0` as non-root (I recommend adding this to your startup script in WSL2, otherwise you will have to run this every time you open WSL2)

#### Windows 11

- install WSLg and install a distribution to it
- follow the steps for Linux -> (the distribution you chose)

### Linux

#### Debian/Ubuntu/Linux mint/PopOS

- run `sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo mtools curl qemu git m4 automake autoconf bash`

#### Fedora/RHEL

- run `sudo dnf install gcc gcc-c++ make bison flex gmp-devel libmpc-devel mpfr-devel texinfo mtools curl qemu git m4 automake autoconf binutils bash`

#### Arch

- run `sudo pacman -Syu base-devel gmp libmpc mpfr mtools curl qemu git bash`

---

## Building

1. run `make` in the appropriate place for your OS (WSL2 for Windows 10, WSLg for Windows 11, etc.). NOTE: If the toolchain and NASM aren't built and installed to the correct location, they **will** be built and installed

## Running - Anything with bash

1. run `./run.sh [config]`. 'config' being either 'debug' or 'release'. If not provided, 'debug' is assumed.

## Running - Other

Run the following command(s) in the appropriate place for your OS (WSL2 for Windows 10 Method 2, cmd/powershell for Windows 10 Method 1, etc.) based on what configuration you want.

### Debug

1. run `qemu-system-x86_64 -pflash ovmf/x86-64/OVMF.fd -hda iso/hdimage.bin -m 256M -debugcon stdio`

### Release

1. run `qemu-system-x86_64 -pflash ovmf/x86-64/OVMF.fd -hda iso/hdimage.bin -m 256M`

---

## Notes

### System Requirement info

- You might be able to run WorldOS with less RAM (by changing the '256M' to the amount of RAM you want), but I cannot guarantee that it would run on any less than 256MiB.
- You computer **must** be capable of virtualization to run WorldOS in a VM.
- You can allocate more than 1 core, but only 1 will be used.

### Program Requirement info

- You can use alternatives to `curl` (such as `wget`), but you will have to modify the `Makefile`.

### Building and Running

- Make sure to follow the setup steps before building and running.

### Other platforms

- All modern POSIX operating systems (including MacOS X) should be able to build and run this OS. It might just take a bit more effort.
- Windows versions that don't support WSL2 or WSLg might require a lot more effort to get this OS running, so I recommend to just install a Linux VM on those.
