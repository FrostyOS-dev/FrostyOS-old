# Copyright (©) 2023  Frosty515
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

include ../Makefile

libc_cxx_source_files := $(shell find src -name *.cpp)
libc_cxx_object_files := $(patsubst src/%.cpp, bin-int/%.cpp.o, $(libc_cxx_source_files))

libc_c_source_files := $(shell find src -name *.c)
libc_c_object_files := $(patsubst src/%.c, bin-int/%.c.o, $(libc_c_source_files))

libc_asm_source_files := $(shell find src -name *.asm)
libc_asm_object_files := $(patsubst src/%.asm, bin-int/%.asm.o, $(libc_asm_source_files))

libc_start_source_files := startfiles/src/arch/x86_64/crt0.asm startfiles/src/arch/x86_64/crti.asm
libc_start_object_files := startfiles/bin-int/arch/x86_64/crt0.o startfiles/bin-int/arch/x86_64/crti.o
libc_end_source_files := startfiles/src/arch/x86_64/crtn.asm
libc_end_object_files := startfiles/bin-int/arch/x86_64/crtn.o

libc_ssp_source_files := ssp/src/stack_protector.c
libc_ssp_object_files := ssp/bin-int/stack_protector.o

LIBC_CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -Wstrict-aliasing -Wunused-parameter -nostdlib -ffreestanding -fstack-protector -fno-exceptions -Isrc -Iinclude -mgeneral-regs-only -mno-red-zone -O2 -fno-use-cxa-atexit -fno-rtti -fno-omit-frame-pointer
LIBC_ASMFLAGS = -f elf64
LIBC_CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -Wstrict-aliasing -Wunused-parameter -nostdlib -ffreestanding -fstack-protector -fno-exceptions -Isrc -Iinclude -mgeneral-regs-only -mno-red-zone -O2 -fno-omit-frame-pointer
LIBC_TARGET = bin/libc.a
LIBSSP_TARGET = bin/libssp_nonshared.a

ifeq ($(config), release)
	LIBC_CXXFLAGS += -DNDEBUG=1
	LIBC_CFLAGS += -DNDEBUG=1
endif

.PHONY: libc clean-libc libc-dependencies install-start-files install-libc install-libc-headers

libc: $(libc_cxx_object_files) $(libc_c_object_files) $(libc_asm_object_files) $(libc_start_object_files) $(libc_end_object_files) $(libc_ssp_object_files)
	@mkdir -p bin
	@echo ------------
	@echo Linking LibC
	@echo ------------
	$(AR) rcs $(LIBC_TARGET) $(libc_asm_object_files) $(libc_c_object_files) $(libc_cxx_object_files) $(libc_ssp_object_files)
	$(AR) rcs $(LIBSSP_TARGET) $(libc_ssp_object_files) bin-int/arch/x86_64/sys/syscall.asm.o

install-start-files: $(libc_start_object_files) $(libc_end_object_files)
	@mkdir -p ../root/data/lib
	@cp startfiles/bin-int/arch/x86_64/crt0.o ../root/data/lib/
	@cp startfiles/bin-int/arch/x86_64/crti.o ../root/data/lib/
	@cp startfiles/bin-int/arch/x86_64/crtn.o ../root/data/lib/

install-libc: install-start-files install-libc-headers
	@mkdir -p ../root/data/lib
	@cp $(LIBC_TARGET) ../root/data/lib/
	@cp $(LIBSSP_TARGET) ../root/data/lib/

install-libc-headers:
	@mkdir -p ../root/data/include
	@cp -fr include/* ../root/data/include/

clean-libc:
	@rm -fr bin-int bin
	@rm -fr startfiles/bin-int startfiles/bin
	@rm -fr ssp/bin-int

distclean-libc: clean-libc
	@rm ../root/data/lib/libc.a ../root/data/lib/crt0.o ../root/data/lib/crti.o ../root/data/lib/crtn.o
	@rm -fr ../root/data/include/*

libc-dependencies:
	@echo 

$(libc_cxx_object_files): bin-int/%.cpp.o : src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -o $@ -c $(patsubst bin-int/%.cpp.o, src/%.cpp, $@) $(LIBC_CXXFLAGS)

$(libc_c_object_files): bin-int/%.c.o : src/%.c
	@mkdir -p $(dir $@)
	$(CC) -o $@ -c $(patsubst bin-int/%.c.o, src/%.c, $@) $(LIBC_CFLAGS)

$(libc_asm_object_files): bin-int/%.asm.o : src/%.asm
	@mkdir -p $(dir $@)
	$(ASM) $(patsubst bin-int/%.asm.o, src/%.asm, $@) $(LIBC_ASMFLAGS) -o $@

$(libc_start_object_files): startfiles/bin-int/%.o : startfiles/src/%.asm
	@mkdir -p $(dir $@)
	$(ASM) $(patsubst startfiles/bin-int/%.o, startfiles/src/%.asm, $@) $(LIBC_ASMFLAGS) -o $@

$(libc_end_object_files): startfiles/bin-int/%.o : startfiles/src/%.asm
	@mkdir -p $(dir $@)
	$(ASM) $(patsubst startfiles/bin-int/%.o, startfiles/src/%.asm, $@) $(LIBC_ASMFLAGS) -o $@

$(libc_ssp_object_files): ssp/bin-int/%.o : ssp/src/%.c
	@mkdir -p $(dir $@)
	$(CC) -o $@ -c $(patsubst ssp/bin-int/%.o, ssp/src/%.c, $@) $(LIBC_CFLAGS)
