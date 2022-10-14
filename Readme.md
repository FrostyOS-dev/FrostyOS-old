# WorldOS

## Latest Changes

### 15/10/2022

- Renamed 'wos-stdint.h' to 'stdint.h'
- Renamed 'wos-stddef.h' to 'stddef.h'
- Renamed 'wos-stdarg.h' to 'stdarg.h'

### 13/10/2022

- Updated this file
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

## Setup

### Windows

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

### Linux

#### Debian/Ubuntu/Linux mint/PopOS

- run `sudo apt install build-essential mtools curl qemu`
- install `mkgpt` from its website/github

## Build Instructions

### Linux

1. run `make boot-iso`

### Windows 10 & Windows 11

1. run `make boot-iso` in WSL2 (Windows 10) or WSLg (Windows 11)

## Running

### Linux

#### Debug:

1. run `qemu-system-x86_64 -pflash ovmf/OVMF.fd -hda iso/hdimage.bin -m 256M -debugcon stdio` in bash or sh

#### Release:

1. run `qemu-system-x86_64 -pflash ovmf/OVMF.fd -hda iso/hdimage.bin -m 256M` in bash or sh

### Windows 10 Method 1

#### Debug:

1. run `qemu-system-x86_64 -pflash ovmf/OVMF.fd -hda iso/hdimage.bin -m 256M -debugcon stdio` from cmd or powershell

#### Release:

1. run `qemu-system-x86_64 -pflash ovmf/OVMF.fd -hda iso/hdimage.bin -m 256M` from cmd or powershell

### Windows 10 Method

#### Debug:

1. Start the X-server and make sure `export DISPLAY = :0` has been run in WSL2
2. run `qemu-system-x86_64 -pflash ovmf/OVMF.fd -hda iso/hdimage.bin -m 256M -debugcon stdio` in WSL2

#### Release:

1. Start the X-server and make sure `export DISPLAY = :0` has been run in WSL2
2. run `qemu-system-x86_64 -pflash ovmf/OVMF.fd -hda iso/hdimage.bin -m 256M` in WSL2

### Windows 11

#### Debug:

1. run `qemu-system-x86_64 -pflash ovmf/OVMF.fd -hda iso/hdimage.bin -m 256M -debugcon stdio` in WSLg

#### Release:

1. run `qemu-system-x86_64 -pflash ovmf/OVMF.fd -hda iso/hdimage.bin -m 256M` in WSLg

## Notes

### Building and Running

- make sure setup steps have been run before building and running

### Other platforms

- All modern Unix based OS's (including MacOS) should be able to build and run this OS. It might just take a bit more effort
- Windows version that don't support WSL2 or WSLg might require a lot more effort to get this OS running, so I recommend to just install a Linux VM on those
