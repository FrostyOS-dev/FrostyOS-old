# WorldOS

## Latest Changes - 25/11/2022

- Switching to C style kernel main, instead of a class function
- Renamed `main.cpp` to `entry.cpp`
- Added Build and run tasks to VS Code config
- Fixed a spelling mistake in `stdbool.h`

## Prerequisites

### Program Requirements

- gcc/g++ version 9 or higher
- mkgpt
- mtools
- qemu version 4 or higher
- curl
- make
- others can be found in [Linux setup section.](#linux)

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
- install an X-server on your Windows 10 host such as VcXsrv
- in WSL2, run `export DISPLAY = :0` as non-root (I recommend adding this to your startup script in WSL2, otherwise you will have to run this every time you open WSL2)

#### Windows 11

- install WSLg and install a distribution to it
- follow the steps for Linux -> (the distribution you chose)

### Linux

#### Debian/Ubuntu/Linux mint/PopOS

- run `sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo mtools curl qemu`
- install `mkgpt` from [its github](https://github.com/jncronin/mkgpt)

#### Fedora/RHEL

- run `sudo dnf install gcc gcc-c++ make bison flex gmp-devel libmpc-devel mpfr-devel texinfo mtools curl qemu`
- install `mkgpt` from [its github](https://github.com/jncronin/mkgpt)

#### Arch

- run `pacman -Syu base-devel gmp libpmc mpfr mtools curl qemu`
- install `mkgpt` from [its github](https://github.com/jncronin/mkgpt)

---

## Building

1. run `make boot-iso` in the appropriate place for your OS (WSL2 for Windows 10, WSLg for Windows 11, etc.). NOTE: If the toolchain and NASM aren't built and installed to the correct location, they **will** be built and installed

## Running - Anything with bash (or similar)

1. run `./run.sh [config]`. 'config' being either 'debug' or 'release'. If not provided, 'debug' is assumed.

## Running - Other

Run the following command(s) in the appropriate place for your OS (WSL2 for Windows 10 Method 2, cmd/powershell for Windows 10 Method 1, etc.) based on what configuration you want.

### Debug

1. run `qemu-system-x86_64 -pflash ovmf/OVMF.fd -hda iso/hdimage.bin -m 256M -debugcon stdio`

### Release

1. run `qemu-system-x86_64 -pflash ovmf/OVMF.fd -hda iso/hdimage.bin -m 256M`

---

## Notes

### System Requirement info

- You might be able to run WorldOS with less RAM (by changing the '256M' to the amount of RAM you want), but I cannot guarantee that it would run on any less than 256MiB.
- You computer **must** be capable of virtualization to run WorldOS in a VM.
- You can allocate more than 1 core, but only 1 will be used.

### Program Requirement info

- You can use alternatives to `mkgpt`, as long as it creates GPT raw disks. You will have to modify the `Makefile` though.
- You can use alternatives to `curl` (such as `wget`), but you will have to modify the `Makefile`.

### Building and Running

- Make sure to follow the setup steps before building and running.

### Other platforms

- All modern POSIX operating systems (including MacOS X) should be able to build and run this OS. It might just take a bit more effort.
- Windows versions that don't support WSL2 or WSLg might require a lot more effort to get this OS running, so I recommend to just install a Linux VM on those.
