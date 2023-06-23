include ../Makefile

kernel_cxx_source_files := $(shell find src -name *.cpp)
kernel_cxx_object_files := $(patsubst src/%.cpp, bin-int/%.cpp.o, $(kernel_cxx_source_files))

kernel_c_source_files := $(shell find src -name *.c)
kernel_c_object_files := $(patsubst src/%.c, bin-int/%.c.o, $(kernel_c_source_files))

kernel_asm_source_files := $(shell find src -name *.asm)
kernel_asm_object_files := $(patsubst src/%.asm, bin-int/%.asm.o, $(kernel_asm_source_files))

KERNEL_CXXFLAGS = -mcmodel=kernel -std=c++20 -g -Wall -Wextra -Wpedantic -Wstrict-aliasing -Wno-pointer-arith -Wno-unused-parameter -nostdlib -nostdinc -ffreestanding -fno-pie -fno-stack-protector -fno-exceptions -Isrc -mgeneral-regs-only -mno-red-zone -O2 -fno-use-cxa-atexit -fno-rtti
KERNEL_LDFLAGS = -Tkernel.ld -static -Bsymbolic -nostdlib -Isrc -zmax-page-size=0x1000
KERNEL_ASMFLAGS = -f elf64
KERNEL_CFLAGS = -mcmodel=kernel -std=c17 -g -Wall -Wextra -Wpedantic -Wstrict-aliasing -Wno-pointer-arith -Wno-unused-parameter -nostdlib -nostdinc -ffreestanding -fno-pie -fno-stack-protector -fno-exceptions -Isrc -mgeneral-regs-only -mno-red-zone -O2
KERNEL_TARGET = bin/kernel.elf

.PHONY: kernel clean-kernel kernel-dependencies

kernel: $(kernel_cxx_object_files) $(kernel_c_object_files) $(kernel_asm_object_files)
	@mkdir -p bin
	@echo --------------
	@echo Linking Kernel
	@echo --------------
	$(LD) $(kernel_asm_object_files) $(kernel_c_object_files) $(kernel_cxx_object_files) -o $(KERNEL_TARGET) $(KERNEL_LDFLAGS)

clean-kernel:
	@rm -fr bin-int
	@rm -fr bin
	@rm -f src/limine.h

kernel-dependencies:
	@curl -o src/limine.h https://raw.githubusercontent.com/limine-bootloader/limine/v4.x-branch-binary/limine.h &> /dev/null

$(kernel_cxx_object_files): bin-int/%.cpp.o : src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) -o $@ -c $(patsubst bin-int/%.cpp.o, src/%.cpp, $@) $(KERNEL_CXXFLAGS)
	@echo 

$(kernel_c_object_files): bin-int/%.c.o : src/%.c
	@mkdir -p $(dir $@)
	$(CC) -o $@ -c $(patsubst bin-int/%.c.o, src/%.c, $@) $(KERNEL_CFLAGS)
	@echo 

$(kernel_asm_object_files): bin-int/%.asm.o : src/%.asm
	@mkdir -p $(dir $@)
	$(ASM) $(patsubst bin-int/%.asm.o, src/%.asm, $@) $(KERNEL_ASMFLAGS) -o $@
	@echo 
