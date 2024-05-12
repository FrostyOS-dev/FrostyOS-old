# Copyright (©) 2022-2024  Frosty515
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

TOOLCHAIN_PREFIX = $(PWD)/toolchain/local

PATH := $(PATH):$(TOOLCHAIN_PREFIX)/bin
SHELL := env PATH=$(PATH) /bin/bash

CC = $(TOOLCHAIN_PREFIX)/bin/x86_64-worldos-gcc
CXX = $(TOOLCHAIN_PREFIX)/bin/x86_64-worldos-g++
LD = $(TOOLCHAIN_PREFIX)/bin/x86_64-worldos-ld
AR = $(TOOLCHAIN_PREFIX)/bin/x86_64-worldos-ar
STRIP = $(TOOLCHAIN_PREFIX)/bin/x86_64-worldos-strip
NM = $(TOOLCHAIN_PREFIX)/bin/x86_64-worldos-nm
ASM = nasm

SYSROOT = $(PWD)/root

export CC CXX LD AR STRIP NM ASM SYSROOT config

all: boot-iso
	@echo --------------
	@echo Build complete
	@echo --------------

run: all
	@echo -------
	@echo Running
	@echo -------
ifeq ($(config), debug)
	@qemu-system-x86_64 -drive if=pflash,file=/usr/share/edk2/x64/OVMF_CODE.fd,format=raw,readonly=on -drive if=pflash,file=ovmf/x86-64/OVMF_VARS.fd,format=raw -drive format=raw,file=iso/hdimage.bin,index=0,media=disk -m 256M -debugcon stdio -machine accel=kvm -M q35 -cpu qemu64
else ifeq ($(config), release)
	@qemu-system-x86_64 -drive if=pflash,file=/usr/share/edk2/x64/OVMF_CODE.fd,format=raw,readonly=on -drive if=pflash,file=ovmf/x86-64/OVMF_VARS.fd,format=raw -drive format=raw,file=iso/hdimage.bin,index=0,media=disk -m 256M -machine accel=kvm -M q35 -cpu qemu64
else
	@qemu-system-x86_64 -drive if=pflash,file=/usr/share/edk2/x64/OVMF_CODE.fd,format=raw,readonly=on -drive if=pflash,file=ovmf/x86-64/OVMF_VARS.fd,format=raw -drive format=raw,file=iso/hdimage.bin,index=0,media=disk -m 256M -debugcon stdio -M q35 -cpu qemu64 -no-reboot -no-shutdown -smp 2 -s
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
	@mkdir -p $(TOOLCHAIN_PREFIX)
ifeq ("$(shell $(TOOLCHAIN_PREFIX)/bin/x86_64-worldos-ld -v 2>/dev/null | grep 2.41)", "")
	@echo -----------------
	@echo Building binutils
	@echo -----------------
	@mkdir -p toolchain/binutils/{src,build}
	@cd toolchain/binutils/src && git clone https://github.com/WorldOS-dev/binutils-gdb.git --depth 1 --branch binutils-2_41-release-point binutils-2.41
	@cd toolchain/binutils/build && CC=gcc CXX=g++ LD=ld AR=ar NM=nm STRIP=strip ASM=as ../src/binutils-2.41/configure --target=x86_64-worldos --prefix="$(TOOLCHAIN_PREFIX)" --with-sysroot=$(SYSROOT) --disable-nls --disable-werror --enable-shared --disable-gdb
	@$(MAKE) -C toolchain/binutils/build -j4
	@$(MAKE) -C toolchain/binutils/build install
	@rm -fr toolchain/binutils
endif
ifneq ("$(shell $(TOOLCHAIN_PREFIX)/bin/x86_64-worldos-gcc -dumpversion 2>/dev/null)", "13.2.1")
	@echo ------------
	@echo Building GCC
	@echo ------------
	@mkdir -p toolchain/gcc/{src,build}
	@cd toolchain/gcc/src && git clone https://github.com/WorldOS-dev/gcc.git --depth 1 --branch releases/gcc-13 gcc-13.2.1
	@cd toolchain/gcc/build && CC=gcc CXX=g++ LD=ld AR=ar NM=nm STRIP=strip ASM=as ../src/gcc-13.2.1/configure --target=x86_64-worldos --prefix="$(TOOLCHAIN_PREFIX)" --with-sysroot=$(SYSROOT) --disable-nls --enable-shared --enable-languages=c,c++
	@$(MAKE) -C toolchain/gcc/build -j4 all-gcc all-target-libgcc
	@$(MAKE) -C toolchain/gcc/build install-gcc install-target-libgcc
	@rm -fr toolchain/gcc
endif

dependencies:
	@mkdir -p dist/boot/EFI/BOOT
	@curl -o dist/boot/EFI/BOOT/BOOTX64.EFI https://raw.githubusercontent.com/limine-bootloader/limine/v6.x-branch-binary/BOOTX64.EFI &> /dev/null
	@mkdir -p depend/tools/bin
ifeq ("$(wildcard depend/tools/bin/mkgpt)","")
	@$(MAKE) mkgpt
endif
ifeq ("$(wildcard ovmf/x86-64/OVMF_VARS.fd)","")
	@mkdir -p ovmf/x86-64
	@cp /usr/share/edk2/x64/OVMF_VARS.fd ovmf/x86-64
endif
	@rm -fr depend/tools/build
	@$(MAKE) -C kernel kernel-dependencies

initramfs:
	@echo ----------------------
	@echo Creating initial RAMFS
	@echo ----------------------
	@mkdir -p dist/boot/WorldOS $(SYSROOT)
	@cd $(SYSROOT) && tar -c --no-auto-compress --owner=0 --group=0 -f ../dist/boot/WorldOS/initramfs.tar *

clean-all:
	@echo ------------
	@echo Cleaning all
	@echo ------------
	@$(MAKE) -C kernel clean-kernel
	@$(MAKE) -C utils clean-utils
	@$(MAKE) -C Userland distclean-userland
	@rm -fr iso dist depend $(SYSROOT)/kernel.map $(SYSROOT)/data/include $(SYSROOT)/data/lib ovmf

clean-os:
	@$(MAKE) -C kernel clean-kernel
	@$(MAKE) -C Userland clean-userland
	@rm -fr iso dist $(SYSROOT)/kernel.map

boot-iso: clean-os .WAIT dependencies toolchain
	@echo ---------------
	@echo Building Kernel
	@echo ---------------
	@$(MAKE) -C kernel kernel
	@echo -------------------------
	@echo Installing Kernel Headers
	@echo -------------------------
	@$(MAKE) -C kernel install-kernel-headers
	@$(MAKE) -C Userland build-userland
	@$(MAKE) -C Userland install-userland
	@$(MAKE) -C utils build
	@$(NM) -C --format=bsd -n kernel/bin/kernel.elf | utils/bin/buildsymboltable $(SYSROOT)/kernel.map
	@$(MAKE) --no-print-directory initramfs
	@echo -----------------
	@echo Making disk image
	@echo -----------------
	@mkdir -p dist/boot/WorldOS
	@cp kernel/bin/kernel.elf dist/boot/WorldOS
	@$(STRIP) --strip-debug dist/boot/WorldOS/kernel.elf
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
