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

utilities_cxx_source_files := $(shell find src -name *.cpp)
utilities_cxx_object_files := $(patsubst src/%.cpp, bin/%, $(utilities_cxx_source_files))

utilities_c_source_files := $(shell find src -name *.c)
utilities_c_object_files := $(patsubst src/%.c, bin/%, $(utilities_c_source_files))

utilities_asm_source_files := $(shell find src -name *.asm)
utilities_asm_object_files := $(patsubst src/%.asm, bin/%, $(utilities_asm_source_files))

UTILITIES_CXXFLAGS = -std=c++20 -Wall -Wextra -fstack-protector -fno-exceptions -mgeneral-regs-only -O2 -fno-use-cxa-atexit -fno-rtti -fno-omit-frame-pointer
UTILITIES_ASMFLAGS = -f elf64
UTILITIES_CFLAGS = -std=c17 -Wall -Wextra -fstack-protector -mgeneral-regs-only -O2 -fno-omit-frame-pointer

ifeq ($(config), release)
	UTILITIES_CXXFLAGS += -DNDEBUG=1
	UTILITIES_CFLAGS += -DNDEBUG=1
endif

.PHONY: build-utilities clean-utilities install-utilities

build-utilities: $(utilities_cxx_object_files) $(utilities_c_object_files) $(utilities_asm_object_files)
	@echo 

install-utilities:
	@mkdir -p $(SYSROOT)/data/bin
	@cp bin/* $(SYSROOT)/data/bin/

clean-utilities:
	@rm -fr bin-int bin

$(utilities_cxx_object_files): bin/% : src/%.cpp
	@mkdir -p bin bin-int
	$(CXX) -c $(patsubst bin/%, src/%.cpp, $@) $(UTILITIES_CXXFLAGS) -o $(patsubst bin/%, bin-int/%.cpp.o, $@)
	$(CC) -o $@ $(patsubst bin/%, bin-int/%.cpp.o, $@)

$(utilities_c_object_files): bin/% : src/%.c
	@mkdir -p bin
	$(CC) -o $@ $(patsubst bin/%, src/%.c, $@) $(UTILITIES_CFLAGS)

$(utilities_asm_object_files): bin/% : src/%.asm
	@mkdir -p bin-int bin
	$(ASM) $(patsubst bin/%, src/%.asm, $@) $(UTILITIES_ASMFLAGS) -o $(patsubst bin/%, bin-int/%.asm.o, $@)
	$(CC) $(patsubst bin/%, bin-int/%.asm.o, $@) -o $@