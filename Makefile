kernel_source_files := $(shell find src/kernel -name *.cpp)
kernel_object_files := $(patsubst src/kernel/%.cpp, bin-int/kernel/%.o, $(kernel_source_files))

kernel_asm_source_files := $(shell find src/kernel -name *.asm)
kernel_asm_object_files := $(patsubst src/kernel/%.asm, bin-int/kernel/%_asm.o, $(kernel_asm_source_files))

$(kernel_object_files): bin-int/kernel/%.o : src/kernel/%.cpp
	mkdir -p $(dir $@)
	g++ -o $@ -c $(patsubst bin-int/kernel/%.o, src/kernel/%.cpp, $@) -m64 -mcmodel=kernel -std=c++20 -g -Wall -Wextra -Wpedantic -Wstrict-aliasing -Wno-pointer-arith -Wno-unused-parameter -nostdlib -nostdinc -ffreestanding -fno-pie -fno-stack-protector -fno-builtin-function -fno-builtin

$(kernel_asm_object_files): bin-int/kernel/%_asm.o : src/kernel/%.asm
	mkdir -p $(dir $@)
	nasm $(patsubst bin-int/kernel/%_asm.o, src/kernel/%.asm, $@) -f elf64 -o $@

.PHONY: boot-iso

cpp-stdlib: $(cpp_stdlib_object_files)
	mkdir -p bin/cpp-stdlib
	$(LD) -shared -Bsymbolic $(cpp_stdlib_object_files) -o bin/cpp-stdlib/cpplib.so

kernel: $(kernel_object_files) $(kernel_asm_object_files)
	mkdir -p bin/kernel
	ld $(kernel_object_files) $(kernel_asm_object_files) -o bin/kernel/kernel.elf -Tsrc/kernel.ld 
	mkdir -p dist/boot/WorldOS
	cp bin/kernel/kernel.elf dist/boot/WorldOS

clean-kernel:
	rm -fr bin-int/kernel
	rm -fr bin/kernel

boot-iso: clean-kernel kernel
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
