# WorldOS

## Latest Changes - 19/10/2022

- Renamed 'Readme.md' to 'README.md'
- Updated Makefile
- Updated this file
- Added `CHANGES.md`

## Prerequisites

### Program Requirements

- gcc/g++ version 10 or higher (any version thats supports C++20 and C17 should work)
- nasm version 2 or higher
- mkgpt
- mtools
- qemu version 4 or higher
- curl
- make

See notes if your system cannot meet these requirements

### System Requirements

- At least 256MiB able to be allocated to QEMU.
- Host that is capable of running x86_64 virtual machines in QEMU.
- Host must be capable of allocating 1 core to a x86_64 QEMU virtual machine.

If you cannot meet these requirements, see notes

### **Windows**

#### Windows 10 Method 1

- install WSL2 and install a distro to it
- follow the steps for Linux -> (distro you chose), except remove `qemu` from the install list (in WSL2)
- install `qemu` from its website and add it to `PATH`

#### Windows 10 Method 2

- install WSL2 and install a distro to it
- follow the steps for Linux -> (distro you chose)
- install an X-server on your Windows 10 host such as VcXsrv
- in WSL2, run `export DISPLAY = :0` as non-root (I recommend adding this to your startup script in WSL2, otherwise you will have to run this everytime you open WSL2)

#### Windows 11

- install WSLg and install a distro to it
- follow the steps for Linux -> (the distro you chose)

### **POSIX** - Linux

#### Debian/Ubuntu/Linux mint/PopOS

- run `sudo apt install build-essential mtools curl qemu nasm`
- install `mkgpt` from [its github](https://github.com/jncronin/mkgpt)

#### Fedora

- run `sudo dnf install gcc gcc-c++ mtools curl qemu nasm make`
- install `mkgpt` from [its github](https://github.com/jncronin/mkgpt)

---

## Building

1. run `make boot-iso` in the appropriate place for your OS (WSL2 for Windows 10, WSLg for Windows 11, etc.)

## Running

Run the following command(s) in the appropriate place for your OS (WSL2 for Windows 10 Method 2, cmd/powershell for Windows 10 Method 1, etc.) based on what configuration you want.

### Debug:

1. run `qemu-system-x86_64 -pflash ovmf/OVMF.fd -hda iso/hdimage.bin -m 256M -debugcon stdio`

### Release:

1. run `qemu-system-x86_64 -pflash ovmf/OVMF.fd -hda iso/hdimage.bin -m 256M`

---

## Notes

### System Requirement info

- You might be able to run WorldOS with less RAM (by changing the '256M' to the amount of RAM you want), but I cannot guarantee that it would run on any less than 256MiB.
- You computer **must** be capable of virtualization to run WorldOS in a VM.
- You can use more than 1 core, but only 1 will be used.

### Program Requirement info

- If the default `gcc/g++` version on your system is not 10 or higher, you might need to modify the `Makefile`.
- You can use alternatives to `mkgpt`, as long as it creates GPT raw disks. You will have to modify the `Makefile` though.
- You can use alternatives to `curl` (such as `wget`), but you will have to modify the `Makefile`.

### Building and Running

- Make sure to follow the setup steps before building and running.

### Other platforms

- All modern POSIX operating systems (including MacOS X) should be able to build and run this OS. It might just take a bit more effort.
- Windows versions that don't support WSL2 or WSLg might require a lot more effort to get this OS running, so I recommend to just install a Linux VM on those.
