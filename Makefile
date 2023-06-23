.PHONY: boot-iso all toolchain dependencies clean-all mkgpt clean-os

TOOLCHAIN_PREFIX = $(HOME)/opt/cross

PATH := $(PATH):$(TOOLCHAIN_PREFIX)/bin
SHELL := env PATH=$(PATH) /bin/bash

CC = x86_64-elf-gcc
CXX = x86_64-elf-g++
LD = x86_64-elf-ld
ASM = nasm

all: boot-iso
	@echo --------------
	@echo Build complete
	@echo --------------

mkgpt:
	@echo --------------
	@echo Building mkgpt
	@echo --------------
	@rm -fr depend/tools/build/mkgpt
	@mkdir -p depend/tools/build/mkgpt
	@curl -OL https://github.com/WorldOS-dev/various-scripts/raw/master/mkgpt/mkgpt.tar.gz
	@tar -xf mkgpt.tar.gz -C depend/tools/build/mkgpt
	@rm -fr mkgpt.tar.gz
	@cd depend/tools/build/mkgpt && ./configure
	@$(MAKE) -C depend/tools/build/mkgpt
	@mkdir -p depend/tools/bin depend/tools/licenses/mkgpt
	@curl -o depend/tools/licenses/mkgpt/LICENSE https://raw.githubusercontent.com/WorldOS-dev/various-scripts/master/mkgpt/LICENSE
	@cp depend/tools/build/mkgpt/mkgpt depend/tools/bin
	@chmod +x depend/tools/bin/mkgpt
	@rm -fr depend/tools/build/mkgpt

toolchain:
ifeq ("$(shell $(TOOLCHAIN_PREFIX)/bin/x86_64-elf-ld -v 2>/dev/null | grep 2.40)", "")
	@echo -----------------
	@echo Building binutils
	@echo -----------------
	@mkdir -p toolchain/binutils/{src,build}
	@curl -o toolchain/binutils/binutils-2.40.tar.gz -L https://ftp.gnu.org/gnu/binutils/binutils-2.40.tar.gz
	@tar -xf toolchain/binutils/binutils-2.40.tar.gz -C toolchain/binutils/src/
	@cd toolchain/binutils/build && ../src/binutils-2.40/configure --target=x86_64-elf --prefix="$(TOOLCHAIN_PREFIX)" --with-sysroot --disable-nls --disable-werror
	@$(MAKE) -C toolchain/binutils/build -j4
	@$(MAKE) -C toolchain/binutils/build install
	@rm -fr toolchain/binutils
endif
ifneq ("$(shell $(TOOLCHAIN_PREFIX)/bin/x86_64-elf-gcc -dumpversion)", "13.1.0")
	@echo ------------
	@echo Building GCC
	@echo ------------
	@mkdir -p toolchain/gcc/{src,build}
	@curl -o toolchain/gcc/gcc-13.1.0.tar.gz -L https://ftp.gnu.org/gnu/gcc/gcc-13.1.0/gcc-13.1.0.tar.gz
	@tar -xf toolchain/gcc/gcc-13.1.0.tar.gz -C toolchain/gcc/src/
	@cd toolchain/gcc && patch -s -p0 < ../../patches/gcc-libgcc-no-red-zone.patch
	@cd toolchain/gcc/build && ../src/gcc-13.1.0/configure --target=x86_64-elf --prefix="$(TOOLCHAIN_PREFIX)" --disable-nls --enable-languages=c,c++ --without-headers
	@$(MAKE) -C toolchain/gcc/build -j4 all-gcc
	@$(MAKE) -C toolchain/gcc/build -j4 all-target-libgcc
	@$(MAKE) -C toolchain/gcc/build install-gcc
	@$(MAKE) -C toolchain/gcc/build install-target-libgcc
endif
	@rm -fr toolchain

dependencies:
	@echo ---------------------
	@echo Fetching dependencies
	@echo ---------------------
	@mkdir -p dist/boot/EFI/BOOT
	@curl -o dist/boot/EFI/BOOT/BOOTX64.EFI https://raw.githubusercontent.com/limine-bootloader/limine/v4.x-branch-binary/BOOTX64.EFI &> /dev/null
	@mkdir -p depend/tools/bin
ifeq ("$(wildcard depend/tools/bin/mkgpt)","")
	@$(MAKE) mkgpt
endif
	@rm -fr depend/tools/build
	@$(MAKE) -C kernel kernel-dependencies

clean-all:
	@echo ------------
	@echo Cleaning all
	@echo ------------
	@$(MAKE) -C kernel clean-kernel
	@rm -fr iso dist depend

clean-os:
	@echo ---------------------------
	@echo Cleaning WorldOS build tree
	@echo ---------------------------
	@$(MAKE) -C kernel clean-kernel
	@rm -fr iso dist

boot-iso: clean-os dependencies toolchain
	@echo ---------------
	@echo Building Kernel
	@echo ---------------
	@$(MAKE) -C kernel kernel
	@echo -----------------
	@echo Making disk image
	@echo -----------------
	@mkdir -p dist/boot/WorldOS
	@cp kernel/bin/kernel.elf dist/boot/WorldOS
	@curl -o dist/boot/limine.cfg https://raw.githubusercontent.com/WorldOS-dev/various-scripts/master/WorldOS/boot/limine.cfg &> /dev/null
	@mkdir -p iso
	@dd if=/dev/zero of=iso/fat.img bs=1k count=1440 &>/dev/null
	@mformat -i iso/fat.img -f 1440 -I 32 ::
	@mmd -i iso/fat.img ::/EFI
	@mmd -i iso/fat.img ::/EFI/BOOT
	@mcopy -i iso/fat.img dist/boot/EFI/BOOT/BOOTX64.EFI ::/EFI/BOOT
	@mmd -i iso/fat.img ::/WorldOS
	@mcopy -i iso/fat.img dist/boot/WorldOS/kernel.elf ::/WorldOS
	@mcopy -i iso/fat.img dist/boot/limine.cfg ::
	@./depend/tools/bin/mkgpt -o iso/hdimage.bin --image-size 8192 --part iso/fat.img --type system
