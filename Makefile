kernel_cxx_source_files := $(shell find src/kernel -name *.cpp)
kernel_cxx_object_files := $(patsubst src/kernel/%.cpp, bin-int/kernel/%.o, $(kernel_cxx_source_files))

kernel_c_source_files := $(shell find src/kernel -name *.c)
kernel_c_object_files := $(patsubst src/kernel/%.c, bin-int/kernel/%_c.o, $(kernel_c_source_files))

kernel_asm_source_files := $(shell find src/kernel -name *.asm)
kernel_asm_object_files := $(patsubst src/kernel/%.asm, bin-int/kernel/%_asm.o, $(kernel_asm_source_files))

KERNEL_CXXFLAGS = -m64 -mcmodel=kernel -std=c++20 -g -Wall -Wextra -Wpedantic -Wstrict-aliasing -Wno-pointer-arith -Wno-unused-parameter -nostdlib -nostdinc -ffreestanding -fno-pie -fno-stack-protector -fno-builtin-function -fno-builtin -fno-exceptions -Isrc/kernel -mgeneral-regs-only -mno-red-zone -O2
KERNEL_CXXC = $(HOME)/opt/cross/bin/x86_64-elf-g++
KERNEL_LD = $(HOME)/opt/cross/bin/x86_64-elf-ld
KERNEL_LDFLAGS = -Tsrc/kernel.ld -static -Bsymbolic -nostdlib -Isrc/kernel -zmax-page-size=0x1000
KERNEL_ASMC = $(HOME)/opt/nasm/bin/nasm
KERNEL_ASMFLAGS = -f elf64
KERNEL_CC = $(HOME)/opt/cross/bin/x86_64-elf-gcc
KERNEL_CFLAGS = -m64 -mcmodel=kernel -std=c17 -g -Wall -Wextra -Wpedantic -Wstrict-aliasing -Wno-pointer-arith -Wno-unused-parameter -nostdlib -nostdinc -ffreestanding -fno-pie -fno-stack-protector -fno-builtin-function -fno-builtin -fno-exceptions -Isrc/kernel -mgeneral-regs-only -mno-red-zone -O2

$(kernel_cxx_object_files): bin-int/kernel/%.o : src/kernel/%.cpp
	@mkdir -p $(dir $@)
	$(KERNEL_CXXC) -o $@ -c $(patsubst bin-int/kernel/%.o, src/kernel/%.cpp, $@) $(KERNEL_CXXFLAGS)

$(kernel_c_object_files): bin-int/kernel/%_c.o : src/kernel/%.c
	@mkdir -p $(dir $@)
	$(KERNEL_CC) -o $@ -c $(patsubst bin-int/kernel/%_c.o, src/kernel/%.c, $@) $(KERNEL_CFLAGS)

$(kernel_asm_object_files): bin-int/kernel/%_asm.o : src/kernel/%.asm
	@mkdir -p $(dir $@)
	$(KERNEL_ASMC) $(patsubst bin-int/kernel/%_asm.o, src/kernel/%.asm, $@) $(KERNEL_ASMFLAGS) -o $@

.PHONY: boot-iso

toolchain:
	@echo ------------------
	@echo Building toolchain
	@echo ------------------
	@chmod +x build-scripts/toolchain-build.sh
	@./build-scripts/toolchain-build.sh

dependencies:
	@echo ---------------------
	@echo Fetching dependencies
	@echo ---------------------
	@mkdir -p dist/boot/EFI/BOOT
	@curl -L -o dist/boot/EFI/BOOT/BOOTX64.EFI https://github.com/limine-bootloader/limine/raw/v4.x-branch-binary/BOOTX64.EFI
	@curl -o src/kernel/limine.h https://raw.githubusercontent.com/limine-bootloader/limine/v4.x-branch-binary/limine.h

kernel: $(kernel_cxx_object_files) $(kernel_c_object_files) $(kernel_asm_object_files)
	@mkdir -p bin/kernel
	@echo --------------
	@echo Linking Kernel
	@echo --------------
	$(KERNEL_LD) $(kernel_asm_object_files) $(kernel_cxx_object_files) $(kernel_c_object_files) -o bin/kernel/kernel.elf $(KERNEL_LDFLAGS)
	@mkdir -p dist/boot/WorldOS
	@cp bin/kernel/kernel.elf dist/boot/WorldOS

clean-kernel:
	@echo ---------------
	@echo Cleaning kernel
	@echo ---------------
	@rm -fr bin-int/kernel
	@rm -fr bin/kernel

clean-all:
	@echo ------------
	@echo Cleaning all
	@echo ------------
	@rm -fr bin
	@rm -fr bin-int
	@rm -fr iso
	@rm -fr dist

boot-iso: clean-all dependencies toolchain
	@echo ---------------
	@echo Building Kernel
	@echo ---------------
	@make kernel
	@echo -----------------
	@echo Making disk image
	@echo -----------------
	@curl -o dist/boot/limine.cfg https://raw.githubusercontent.com/WorldOS-dev/various-scripts/master/WorldOS/boot/limine.cfg
	@mkdir -p iso
	dd if=/dev/zero of=iso/fat.img bs=1k count=1440
	mformat -i iso/fat.img -f 1440 -I 32 ::
	mmd -i iso/fat.img ::/EFI
	mmd -i iso/fat.img ::/EFI/BOOT
	mcopy -i iso/fat.img dist/boot/EFI/BOOT/BOOTX64.EFI ::/EFI/BOOT
	mmd -i iso/fat.img ::/WorldOS
	mcopy -i iso/fat.img dist/boot/WorldOS/kernel.elf ::/WorldOS
	mcopy -i iso/fat.img dist/boot/limine.cfg ::
	mkgpt -o iso/hdimage.bin --image-size 8192 --part iso/fat.img --type system
