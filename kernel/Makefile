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

kernel_cxx_source_files := $(shell find src -name *.cpp)
kernel_cxx_object_files := $(patsubst src/%.cpp, bin-int/%.cpp.o, $(kernel_cxx_source_files))

kernel_c_source_files := $(shell find src -name *.c)
kernel_c_object_files := $(patsubst src/%.c, bin-int/%.c.o, $(kernel_c_source_files))

kernel_asm_source_files := $(shell find src -name *.asm)
kernel_asm_object_files := $(patsubst src/%.asm, bin-int/%.asm.o, $(kernel_asm_source_files))

kernel_lib_cxx_source_files := $(shell find lib/src -name *.cpp)
kernel_lib_cxx_object_files := $(patsubst lib/src/%.cpp, lib/bin-int/%.cpp.o, $(kernel_lib_cxx_source_files))

kernel_lib_c_source_files := $(shell find lib/src -name *.c)
kernel_lib_c_object_files := $(patsubst lib/src/%.c, lib/bin-int/%.c.o, $(kernel_lib_c_source_files))

kernel_lib_asm_source_files := $(shell find lib/src -name *.asm)
kernel_lib_asm_object_files := $(patsubst lib/src/%.asm, lib/bin-int/%.asm.o, $(kernel_lib_asm_source_files))

KERNEL_CXXFLAGS = -mcmodel=kernel -std=c++20 -g -Wall -Wextra -Wpedantic -Wstrict-aliasing -Wno-pointer-arith -Wno-unused-parameter -Wno-switch -nostdlib -ffreestanding -fno-pie -fstack-protector -fno-exceptions -Isrc -Ilib/include -Iheaders -mgeneral-regs-only -mno-red-zone -O2 -fno-use-cxa-atexit -fno-rtti -fno-omit-frame-pointer -D_IN_KERNEL=1
KERNEL_LDFLAGS = -Tkernel.ld -static -Bsymbolic -nostdlib -zmax-page-size=0x1000
KERNEL_ASMFLAGS = -f elf64
KERNEL_CFLAGS = -mcmodel=kernel -std=c17 -g -Wall -Wextra -Wpedantic -Wstrict-aliasing -Wno-pointer-arith -Wno-unused-parameter -Wno-switch -nostdlib -ffreestanding -fno-pie -fstack-protector -fno-exceptions -Isrc -Ilib/include -Iheaders -mgeneral-regs-only -mno-red-zone -O2 -fno-omit-frame-pointer -D_IN_KERNEL=1
KERNEL_TARGET = bin/kernel.elf
KERNEL_LIB_TARGET = lib/bin/kernel.a

ifeq ($(config), release)
	KERNEL_CXXFLAGS += -DNDEBUG=1
	KERNEL_CFLAGS += -DNDEBUG=1
endif

.PHONY: kernel clean-kernel kernel-dependencies install-kernel-headers kernel-lib

kernel: $(kernel_cxx_object_files) $(kernel_c_object_files) $(kernel_asm_object_files) kernel-lib
	@mkdir -p bin
	@echo --------------
	@echo Linking Kernel
	@echo --------------
	$(LD) $(kernel_asm_object_files) $(kernel_c_object_files) $(kernel_cxx_object_files) $(KERNEL_LIB_TARGET) -o $(KERNEL_TARGET) $(KERNEL_LDFLAGS)

kernel-lib: $(kernel_lib_cxx_object_files) $(kernel_lib_c_object_files) $(kernel_lib_asm_object_files)
	@mkdir -p lib/bin
	@echo ------------------
	@echo Linking Kernel Lib
	@echo ------------------
	$(AR) rcs $(KERNEL_LIB_TARGET) $(kernel_lib_asm_object_files) $(kernel_lib_c_object_files) $(kernel_lib_cxx_object_files)

clean-kernel:
	@rm -fr bin-int
	@rm -fr bin
	@rm -fr lib/bin-int
	@rm -fr lib/bin
	@rm -f src/limine.h

kernel-dependencies:
	@curl -o src/limine.h https://raw.githubusercontent.com/limine-bootloader/limine/v6.x-branch-binary/limine.h &> /dev/null

install-kernel-headers:
	@mkdir -p ../root/data/include/kernel
	@cp -r headers/* ../root/data/include/kernel/

$(kernel_cxx_object_files): bin-int/%.cpp.o : src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -o $@ -c $(patsubst bin-int/%.cpp.o, src/%.cpp, $@) $(KERNEL_CXXFLAGS)

$(kernel_c_object_files): bin-int/%.c.o : src/%.c
	@mkdir -p $(dir $@)
	$(CC) -o $@ -c $(patsubst bin-int/%.c.o, src/%.c, $@) $(KERNEL_CFLAGS)

$(kernel_asm_object_files): bin-int/%.asm.o : src/%.asm
	@mkdir -p $(dir $@)
	$(ASM) $(patsubst bin-int/%.asm.o, src/%.asm, $@) $(KERNEL_ASMFLAGS) -o $@

$(kernel_lib_cxx_object_files): lib/bin-int/%.cpp.o : lib/src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -o $@ -c $(patsubst lib/bin-int/%.cpp.o, lib/src/%.cpp, $@) $(KERNEL_CXXFLAGS)

$(kernel_lib_c_object_files): lib/bin-int/%.c.o : lib/src/%.c
	@mkdir -p $(dir $@)
	$(CC) -o $@ -c $(patsubst lib/bin-int/%.c.o, lib/src/%.c, $@) $(KERNEL_CFLAGS)

$(kernel_lib_asm_object_files): lib/bin-int/%.asm.o : lib/src/%.asm
	@mkdir -p $(dir $@)
	$(ASM) $(patsubst lib/bin-int/%.asm.o, lib/src/%.asm, $@) $(KERNEL_ASMFLAGS) -o $@
