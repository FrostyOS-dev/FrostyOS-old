kernel_cxx_source_files := $(shell find src/kernel -name *.cpp)
kernel_cxx_object_files := $(patsubst src/kernel/%.cpp, bin-int/kernel/%.o, $(kernel_cxx_source_files))

kernel_c_source_files := $(shell find src/kernel -name *.c)
kernel_c_object_files := $(patsubst src/kernel/%.c, bin-int/kernel/%_c.o, $(kernel_c_source_files))

kernel_asm_source_files := $(shell find src/kernel -name *.asm)
kernel_asm_object_files := $(patsubst src/kernel/%.asm, bin-int/kernel/%_asm.o, $(kernel_asm_source_files))

KERNEL_CXXFLAGS = -m64 -mcmodel=kernel -std=c++20 -g -Wall -Wextra -Wpedantic -Wstrict-aliasing -Wno-pointer-arith -Wno-unused-parameter -nostdlib -nostdinc -ffreestanding -fno-pie -fno-stack-protector -fno-builtin-function -fno-builtin -fno-exceptions -Isrc/kernel -mgeneral-regs-only -mno-red-zone -O2
KERNEL_CXXC = g++-10
KERNEL_LD = ld
KERNEL_LDFLAGS = -Tsrc/kernel.ld -static -Bsymbolic -nostdlib -Isrc/kernel
KERNEL_ASMC = nasm
KERNEL_ASMFLAGS = -f elf64
KERNEL_CC = gcc-10
KERNEL_CFLAGS = -m64 -mcmodel=kernel -std=c17 -g -Wall -Wextra -Wpedantic -Wstrict-aliasing -Wno-pointer-arith -Wno-unused-parameter -nostdlib -nostdinc -ffreestanding -fno-pie -fno-stack-protector -fno-builtin-function -fno-builtin -fno-exceptions -Isrc/kernel -mgeneral-regs-only -mno-red-zone -O2

$(kernel_cxx_object_files): bin-int/kernel/%.o : src/kernel/%.cpp
	mkdir -p $(dir $@)
	$(KERNEL_CXXC) -o $@ -c $(patsubst bin-int/kernel/%.o, src/kernel/%.cpp, $@) $(KERNEL_CXXFLAGS)

$(kernel_c_object_files): bin-int/kernel/%_c.o : src/kernel/%.c
	mkdir -p $(dir $@)
	$(KERNEL_CC) -o $@ -c $(patsubst bin-int/kernel/%_c.o, src/kernel/%.c, $@) $(KERNEL_CFLAGS)

$(kernel_asm_object_files): bin-int/kernel/%_asm.o : src/kernel/%.asm
	mkdir -p $(dir $@)
	$(KERNEL_ASMC) $(patsubst bin-int/kernel/%_asm.o, src/kernel/%.asm, $@) $(KERNEL_ASMFLAGS) -o $@

.PHONY: boot-iso

dependencies:
	mkdir -p dist/boot/EFI/BOOT
	curl -L -o dist/boot/EFI/BOOT/BOOTX64.EFI https://github.com/limine-bootloader/limine/raw/v4.20220928.0-binary/BOOTX64.EFI

kernel: $(kernel_cxx_object_files) $(kernel_c_object_files) $(kernel_asm_object_files)
	mkdir -p bin/kernel
	$(KERNEL_LD)  $(kernel_asm_object_files) $(kernel_cxx_object_files) $(kernel_c_object_files) -o bin/kernel/kernel.elf $(KERNEL_LDFLAGS)

clean-kernel:
	rm -fr bin-int/kernel
	rm -fr bin/kernel

clean-all:
	rm -fr bin
	rm -fr bin-int
	rm -fr iso
	rm -fr dist

boot-iso: clean-all dependencies kernel
	mkdir -p dist/boot/WorldOS
	cp bin/kernel/kernel.elf dist/boot/WorldOS
	curl -o dist/boot/limine.cfg https://raw.githubusercontent.com/WorldOS-dev/various-scripts/master/WorldOS/boot/limine.cfg
	mkdir -p iso
	dd if=/dev/zero of=iso/fat.img bs=1k count=1440
	mformat -i iso/fat.img -f 1440 ::
	mmd -i iso/fat.img ::/EFI
	mmd -i iso/fat.img ::/EFI/BOOT
	mcopy -i iso/fat.img dist/boot/EFI/BOOT/BOOTX64.EFI ::/EFI/BOOT
	mmd -i iso/fat.img ::/WorldOS
	mcopy -i iso/fat.img dist/boot/WorldOS/kernel.elf ::/WorldOS
	mcopy -i iso/fat.img dist/boot/limine.cfg ::
	mkgpt -o iso/hdimage.bin --image-size 8192 --part iso/fat.img --type system
