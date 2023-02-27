kernel_cxx_source_files := $(shell find kernel/src -name *.cpp)
kernel_cxx_object_files := $(patsubst kernel/src/%.cpp, bin-int/kernel/%.o, $(kernel_cxx_source_files))

kernel_c_source_files := $(shell find kernel/src -name *.c)
kernel_c_object_files := $(patsubst kernel/src/%.c, bin-int/kernel/%_c.o, $(kernel_c_source_files))

kernel_asm_source_files := $(shell find kernel/src -name *.asm)
kernel_asm_object_files := $(patsubst kernel/src/%.asm, bin-int/kernel/%_asm.o, $(kernel_asm_source_files))

KERNEL_CXXFLAGS = -mcmodel=kernel -std=c++20 -g -Wall -Wextra -Wpedantic -Wstrict-aliasing -Wno-pointer-arith -Wno-unused-parameter -nostdlib -nostdinc -ffreestanding -fno-pie -fno-stack-protector -fno-exceptions -Ikernel/src -mgeneral-regs-only -mno-red-zone -O2 -fno-use-cxa-atexit -fno-rtti
KERNEL_CXXC = $(HOME)/opt/cross/bin/x86_64-elf-g++
KERNEL_LD = $(HOME)/opt/cross/bin/x86_64-elf-ld
KERNEL_LDFLAGS = -Tkernel/kernel.ld -static -Bsymbolic -nostdlib -Ikernel/src -zmax-page-size=0x1000
KERNEL_ASMC = nasm
KERNEL_ASMFLAGS = -f elf64
KERNEL_CC = $(HOME)/opt/cross/bin/x86_64-elf-gcc
KERNEL_CFLAGS = -mcmodel=kernel -std=c17 -g -Wall -Wextra -Wpedantic -Wstrict-aliasing -Wno-pointer-arith -Wno-unused-parameter -nostdlib -nostdinc -ffreestanding -fno-pie -fno-stack-protector -fno-exceptions -Ikernel/src -mgeneral-regs-only -mno-red-zone -O2

.PHONY: boot-iso all toolchain dependencies kernel clean-kernel clean-all mkgpt clean-os

SHELL := /bin/bash

all: boot-iso
	@echo --------------
	@echo Build complete
	@echo --------------

$(kernel_cxx_object_files): bin-int/kernel/%.o : kernel/src/%.cpp
	@mkdir -p $(dir $@)
	$(KERNEL_CXXC) -o $@ -c $(patsubst bin-int/kernel/%.o, kernel/src/%.cpp, $@) $(KERNEL_CXXFLAGS)
	@echo 

$(kernel_c_object_files): bin-int/kernel/%_c.o : kernel/src/%.c
	@mkdir -p $(dir $@)
	$(KERNEL_CC) -o $@ -c $(patsubst bin-int/kernel/%_c.o, kernel/src/%.c, $@) $(KERNEL_CFLAGS)
	@echo 

$(kernel_asm_object_files): bin-int/kernel/%_asm.o : kernel/src/%.asm
	@mkdir -p $(dir $@)
	$(KERNEL_ASMC) $(patsubst bin-int/kernel/%_asm.o, kernel/src/%.asm, $@) $(KERNEL_ASMFLAGS) -o $@
	@echo 

mkgpt:
	@echo --------------
	@echo Building mkgpt
	@echo --------------
	@rm -fr depend/tools/build/mkgpt
	@mkdir -p depend/tools/build/mkgpt
	@curl -OL https://github.com/WorldOS-dev/various-scripts/raw/master/mkgpt/mkgpt.tar.gz
	@cd depend/tools/build/mkgpt && tar -xf ../../../../mkgpt.tar.gz
	@rm -fr mkgpt.tar.gz
	@cd depend/tools/build/mkgpt && ./configure
	@$(MAKE) -C depend/tools/build/mkgpt
	@mkdir -p depend/tools/bin depend/tools/licenses/mkgpt
	@curl -o depend/tools/licenses/mkgpt/LICENSE https://raw.githubusercontent.com/WorldOS-dev/various-scripts/master/mkgpt/LICENSE
	@cp depend/tools/build/mkgpt/mkgpt depend/tools/bin
	@chmod +x depend/tools/bin/mkgpt
	@rm -fr depend/tools/build/mkgpt

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
	@curl -o kernel/src/limine.h https://raw.githubusercontent.com/limine-bootloader/limine/v4.x-branch-binary/limine.h
	@mkdir -p depend/tools/bin
ifeq ("$(wildcard depend/tools/bin/mkgpt)","")
	@$(MAKE) mkgpt
endif
	@rm -fr depend/tools/build

kernel: $(kernel_cxx_object_files) $(kernel_c_object_files) $(kernel_asm_object_files)
	@mkdir -p bin/kernel
	@echo --------------
	@echo Linking Kernel
	@echo --------------
	$(KERNEL_LD) $(kernel_asm_object_files) $(kernel_c_object_files) $(kernel_cxx_object_files) -o bin/kernel/kernel.elf $(KERNEL_LDFLAGS)
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
	@rm -fr depend
	@rm kernel/src/limine.h

clean-os:
	@echo ---------------------------
	@echo Cleaning WorldOS build tree
	@echo ---------------------------
	@rm -fr bin bin-int iso dist

boot-iso: clean-os dependencies toolchain
	@echo ---------------
	@echo Building Kernel
	@echo ---------------
	@$(MAKE) kernel
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
	./depend/tools/bin/mkgpt -o iso/hdimage.bin --image-size 8192 --part iso/fat.img --type system
