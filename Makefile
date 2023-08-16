# Copyright (©) 2022-2023  Frosty515
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

.PHONY: boot-iso all toolchain dependencies clean-all mkgpt clean-os run initramfs

ifndef config
	config=debug
endif

TOOLCHAIN_PREFIX = $(HOME)/opt/x86_64-worldos-cross

PATH := $(PATH):$(TOOLCHAIN_PREFIX)/bin
SHELL := env PATH=$(PATH) /bin/bash

CC = $(TOOLCHAIN_PREFIX)/bin/x86_64-worldos-gcc
CXX = $(TOOLCHAIN_PREFIX)/bin/x86_64-worldos-g++
LD = $(TOOLCHAIN_PREFIX)/bin/x86_64-worldos-ld
ASM = nasm

all: boot-iso
	@echo --------------
	@echo Build complete
	@echo --------------

run: all
	@echo -------
	@echo Running
	@echo -------
ifeq ($(config), debug)
	@qemu-system-x86_64 -drive if=pflash,file=ovmf/x86-64/OVMF.fd,format=raw -drive format=raw,file=iso/hdimage.bin,if=none,id=nvm -device nvme,serial=deadbeef,drive=nvm,use-intel-id=on -m 256M -debugcon stdio -machine accel=kvm -M q35 -cpu qemu64
else ifeq ($(config), release)
	@qemu-system-x86_64 -drive if=pflash,file=ovmf/x86-64/OVMF.fd,format=raw -drive format=raw,file=iso/hdimage.bin,if=none,id=nvm -device nvme,serial=deadbeef,drive=nvm,use-intel-id=on -m 256M -machine accel=kvm -M q35 -cpu qemu64
else
	@qemu-system-x86_64 -drive if=pflash,file=ovmf/x86-64/OVMF.fd,format=raw -drive format=raw,file=iso/hdimage.bin,if=none,id=nvm -device nvme,serial=deadbeef,drive=nvm,use-intel-id=on -m 256M -debugcon stdio -machine accel=kvm -M q35 -cpu qemu64
endif

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
ifeq ("$(shell $(TOOLCHAIN_PREFIX)/bin/x86_64-worldos-ld -v 2>/dev/null | grep 2.41)", "")
	@echo -----------------
	@echo Building binutils
	@echo -----------------
	@mkdir -p toolchain/binutils/{src,build}
	@curl -o toolchain/binutils/binutils-2.41.tar.xz -L https://ftp.gnu.org/gnu/binutils/binutils-2.41.tar.xz
	@tar -xJf toolchain/binutils/binutils-2.41.tar.xz -C toolchain/binutils/src/
	@cd toolchain/binutils/src && patch -s -p0 < ../../../patches/binutils.patch
	@cd toolchain/binutils/build && ../src/binutils-2.41/configure --target=x86_64-worldos --prefix="$(TOOLCHAIN_PREFIX)" --with-sysroot=$(PWD)/root --disable-nls --disable-werror --enable-shared
	@$(MAKE) -C toolchain/binutils/build -j4
	@$(MAKE) -C toolchain/binutils/build install
	@rm -fr toolchain/binutils
endif
ifneq ("$(shell $(TOOLCHAIN_PREFIX)/bin/x86_64-worldos-gcc -dumpversion)", "13.2.0")
	@echo ------------
	@echo Building GCC
	@echo ------------
	@mkdir -p toolchain/gcc/{src,build}
	@curl -o toolchain/gcc/gcc-13.2.0.tar.xz -L https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.xz
	@tar -xJf toolchain/gcc/gcc-13.2.0.tar.xz -C toolchain/gcc/src/
	@cd toolchain/gcc/src && patch -s -p0 < ../../../patches/gcc.patch
	@cd toolchain/gcc/build && ../src/gcc-13.2.0/configure --target=x86_64-worldos --prefix="$(TOOLCHAIN_PREFIX)" --with-sysroot=$(PWD)/root --disable-nls --enable-shared --enable-languages=c,c++
	@$(MAKE) -C toolchain/gcc/build -j4 all-gcc all-target-libgcc
	@$(MAKE) -C toolchain/gcc/build install-gcc install-target-libgcc
endif
	@rm -fr toolchain

dependencies:
	@mkdir -p dist/boot/EFI/BOOT
	@curl -o dist/boot/EFI/BOOT/BOOTX64.EFI https://raw.githubusercontent.com/limine-bootloader/limine/v5.x-branch-binary/BOOTX64.EFI &> /dev/null
	@mkdir -p depend/tools/bin
ifeq ("$(wildcard depend/tools/bin/mkgpt)","")
	@$(MAKE) mkgpt
endif
	@rm -fr depend/tools/build
	@$(MAKE) -C kernel kernel-dependencies

initramfs:
	@echo ----------------------
	@echo Creating initial RAMFS
	@echo ----------------------
	@mkdir -p dist/boot/WorldOS root
	@cd root && tar -c --no-auto-compress -f ../dist/boot/WorldOS/initramfs.tar *

clean-all:
	@echo ------------
	@echo Cleaning all
	@echo ------------
	@$(MAKE) -C kernel clean-kernel
	@$(MAKE) -C utils clean-utils
	@rm -fr iso dist depend root/kernel.map

clean-os:
	@$(MAKE) -C kernel clean-kernel
	@rm -fr iso dist root/kernel.map

boot-iso: clean-os .WAIT dependencies toolchain
	@echo ---------------
	@echo Building Kernel
	@echo ---------------
	@$(MAKE) -C kernel kernel config=$(config)
	@echo --------------
	@echo Building utils
	@echo --------------
	@$(MAKE) -C utils build
	@utils/bin/buildsymboltable kernel/bin/kernel.elf root/kernel.map
	@$(MAKE) --no-print-directory initramfs
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
	@mcopy -i iso/fat.img dist/boot/WorldOS/initramfs.tar ::/WorldOS
	@mcopy -i iso/fat.img dist/boot/limine.cfg ::
	@./depend/tools/bin/mkgpt -o iso/hdimage.bin --image-size 8192 --part iso/fat.img --type system
