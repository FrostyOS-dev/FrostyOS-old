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

programs_cxx_source_files := $(shell find src -name *.cpp)
programs_cxx_object_files := $(patsubst src/%.cpp, bin/%, $(programs_cxx_source_files))

programs_c_source_files := $(shell find src -name *.c)
programs_c_object_files := $(patsubst src/%.c, bin/%, $(programs_c_source_files))

programs_asm_source_files := $(shell find src -name *.asm)
programs_asm_object_files := $(patsubst src/%.asm, bin/%, $(programs_asm_source_files))

PROGRAMS_CXXFLAGS = -std=c++20 -Wall -Wextra -fstack-protector -fno-exceptions -mgeneral-regs-only -O2 -fno-use-cxa-atexit -fno-rtti -fno-omit-frame-pointer
PROGRAMS_ASMFLAGS = -f elf64
PROGRAMS_CFLAGS = -std=c17 -Wall -Wextra -fstack-protector -mgeneral-regs-only -O2 -fno-omit-frame-pointer

ifeq ($(config), release)
	PROGRAMS_CXXFLAGS += -DNDEBUG=1
	PROGRAMS_CFLAGS += -DNDEBUG=1
endif

.PHONY: programs clean-programs install-programs

programs: $(programs_cxx_object_files) $(programs_c_object_files) $(programs_asm_object_files)
	@echo 

install-programs:
	@mkdir -p ../root/data/bin
	@cp bin/* ../root/data/bin/

clean-programs:
	@rm -fr bin-int bin

$(programs_cxx_object_files): bin/% : src/%.cpp
	@mkdir -p bin bin-int
	$(CXX) -c $(patsubst bin/%, src/%.cpp, $@) $(PROGRAMS_CXXFLAGS) -o $(patsubst bin/%, bin-int/%.cpp.o, $@)
	$(CC) -o $@ $(patsubst bin/%, bin-int/%.cpp.o, $@)

$(programs_c_object_files): bin/% : src/%.c
	@mkdir -p bin
	$(CC) -o $@ $(patsubst bin/%, src/%.c, $@) $(PROGRAMS_CFLAGS)

$(programs_asm_object_files): bin/% : src/%.asm
	@mkdir -p bin-int bin
	$(ASM) $(patsubst bin/%, src/%.asm, $@) $(PROGRAMS_ASMFLAGS) -o $(patsubst bin/%, bin-int/%.asm.o, $@)
	$(CC) $(patsubst bin/%, bin-int/%.asm.o, $@) -o $@