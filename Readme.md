# WorldOS Main

## Setup

### Windows

#### Windows 10

- install WSL2 and install a distro to it
- follow the steps for Linux -> (distro you chose), except remove `qemu` from the install list (in WSL2)
- install `qemu` from its website and add it to `PATH`

#### Windows 11

- install WSLg and install a distro to it
- follow the steps for Linux -> (the distro you chose)

### Linux

#### Debian/Ubuntu/Linux mint/PopOS

- run `sudo apt install build-essential mtools qemu`
- install `mkgpt` from its website/github

## Build Instructions

### Linux

1. run `make boot-iso`
2. run `qemu-system-x86_64 -pflash ovmf/OVMF.fd -hda iso/hdimage.bin -m 256M`

### Windows 10

1. run `make boot-iso` in WSL2
2. run `qemu-system-x86_64 -pflash ovmf/OVMF.fd -hda iso/hdimage.bin` from cmd or powershell

### Windows 11

1. run `make boot-iso` in WSLg
2. run `qemu-system-x86_64 -pflash ovmf/OVMF.fd -hda iso/hdimage.bin` in WSLg

**OR**

follow the same steps as Windows 10

### Other

Unsupported

## Notes

### Building and Running

- qemu-system-x86_64 is required
- memory utils such as `mmd` and `mcopy` must be installed
- latest gnu-gcc must be installed
- mkgpt must be installed