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

cxx_source_files := $(shell find src -name *.cpp)
cxx_object_files := $(patsubst src/%.cpp, bin/%, $(cxx_source_files))

c_source_files := $(shell find src -name *.c)
c_object_files := $(patsubst src/%.c, bin/%, $(c_source_files))

asm_source_files := $(shell find src -name *.asm)
asm_object_files := $(patsubst src/%.asm, bin/%, $(asm_source_files))

CXX = g++
CC = gcc
LD = ld
ASM = nasm

CXXFLAGS = -std=c++20 -g -Wall -Wextra -fno-exceptions -mgeneral-regs-only -O2 -Wno-write-strings
LDFLAGS = 
ASMFLAGS = -f elf64
CFLAGS = -std=c17 -g -Wall -Wextra -fno-exceptions -mgeneral-regs-only -O2

.PHONY: build clean-utils

build: $(cxx_object_files) $(c_object_files) $(asm_object_files)


clean-utils:
	@rm -fr bin

$(cxx_object_files): bin/% : src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -o $@ $(patsubst bin/%, src/%.cpp, $@) $(CXXFLAGS)

$(c_object_files): bin/% : src/%.c
	@mkdir -p $(dir $@)
	$(CC) -o $@ $(patsubst bin/%, src/%.c, $@) $(CFLAGS)

$(asm_object_files): bin/% : src/%.asm
	@mkdir -p $(dir $@)
	$(ASM) $(patsubst bin/%, src/%.asm, $@) $(ASMFLAGS) -o $@
	$(LD) $@ -o $@
