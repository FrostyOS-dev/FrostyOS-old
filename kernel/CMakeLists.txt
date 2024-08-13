# Copyright (©) 2024  Frosty515

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

include(dependencies/uACPI/uacpi.cmake)

if (FROSTYOS_BUILD_TARGET STREQUAL "kernel")
    add_library(uACPI STATIC ${UACPI_SOURCES})

    target_include_directories(uACPI PRIVATE ${UACPI_INCLUDES})

    set_target_properties(uACPI PROPERTIES CXX_STANDARD 23)
    set_target_properties(uACPI PROPERTIES CXX_STANDARD_REQUIRED ON)
    set_target_properties(uACPI PROPERTIES CXX_EXTENSIONS OFF)
    set_target_properties(uACPI PROPERTIES C_STANDARD 23)
    set_target_properties(uACPI PROPERTIES C_EXTENSIONS OFF)
    set_target_properties(uACPI PROPERTIES C_STANDARD_REQUIRED ON)


    if (FROSTYOS_ENABLE_KASAN STREQUAL "yes")
        target_compile_definitions(uACPI PRIVATE _FROSTYOS_ENABLE_KASAN=1)

        target_compile_options(uACPI
            PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fsanitize=kernel-address,undefined>
        )
    else()
        target_compile_definitions(uACPI PRIVATE _FROSTYOS_ENABLE_KASAN=0)
        
        target_compile_options(uACPI
            PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fsanitize=undefined>
        )
    endif()

    target_compile_options(uACPI
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-mcmodel=kernel>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-g>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wall>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wextra>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wno-pedantic>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wno-strict-aliasing>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wno-pointer-arith>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wno-unused-parameter>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wno-switch>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wno-packed-bitfield-compat>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wno-maybe-uninitialized>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-nostdlib>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-ffreestanding>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fno-pie>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fstack-protector>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-mgeneral-regs-only>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-mno-red-zone>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-O2>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fno-omit-frame-pointer>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    )


endif()

if(FROSTYOS_ARCHITECTURE STREQUAL "x86_64")
    if (FROSTYOS_BUILD_TARGET STREQUAL "kernel")
        set(kernel_sources
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/interrupts/IDT.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/interrupts/isrs.asm
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/interrupts/isrs_gen.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/interrupts/pic.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/interrupts/APIC/IOAPIC.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/interrupts/APIC/IPI.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/interrupts/APIC/LocalAPIC.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/interrupts/IRQ.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/interrupts/isr.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/interrupts/NMI.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/Memory/PagingUtil.asm
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/Memory/PagingUtil.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/Memory/PageMapIndexer.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/Memory/PagingInit.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/Memory/PAT.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/Scheduling/syscall.asm
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/Scheduling/task.asm
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/Scheduling/taskutil.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/Scheduling/taskutil.asm
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/CMOS.asm
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/E9.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/GDT.asm
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/RTC.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/TSS.asm
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/cpuid.asm
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/entry.asm
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/io.asm
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/panic.asm
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/ELFKernel.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/ELFSymbols.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/Stack.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/8042PS2Controller.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/APStartup.asm
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/GDT.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/PIT.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/Processor.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/panic.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/MSR.asm
        )
    endif()

    if (FROSTYOS_BUILD_TARGET STREQUAL "Userland")
        set(kernel_sources
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/ELFSymbols.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/UserCalls.asm
            ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/UserEntry.asm
        )
    endif()

    if (FROSTYOS_ENABLE_KASAN STREQUAL "yes")
        set(kernel_lib_sources
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/src/arch/x86_64/util-kasan.asm
        )
    else()
        set(kernel_lib_sources
            ${CMAKE_CURRENT_SOURCE_DIR}/lib/src/arch/x86_64/util.asm
        )
    endif()

    set(kernel_lib_sources
        ${kernel_lib_sources}
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/src/arch/x86_64/math.asm
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/src/arch/x86_64/spinlock.asm
    )
endif()

if (FROSTYOS_BUILD_TARGET STREQUAL "kernel")
    if(FROSTYOS_ENABLE_KASAN STREQUAL "yes")
        set(kernel_sources
            ${kernel_sources}
            ${CMAKE_CURRENT_SOURCE_DIR}/src/sanitiser/asan.cpp
        )
    endif()

    set(kernel_sources
        ${kernel_sources}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics/VGA.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics/VGAFont.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/HAL/drivers/ACPI/ACPI.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/HAL/drivers/ACPI/ACPIDevice.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/HAL/drivers/ACPI/FADT.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/HAL/drivers/ACPI/HPET.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/HAL/drivers/ACPI/MADT.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/HAL/drivers/ACPI/MCFG.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/HAL/drivers/ACPI/uACPI.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/HAL/drivers/HPET.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/HAL/drivers/Keyboard.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/HAL/drivers/PCI.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/HAL/drivers/PS2/PS2Controller.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/HAL/drivers/PS2/PS2Keyboard.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Memory/PhysicalPageFrameAllocator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Memory/Stack.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Memory/VirtualPageManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Memory/PageManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Memory/PageFault.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Scheduling/Process.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Scheduling/Semaphore.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Scheduling/Scheduler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Scheduling/Thread.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/SystemCalls/exec.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/SystemCalls/exit.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/SystemCalls/memory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/SystemCalls/mount.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/SystemCalls/Synchronisation.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/SystemCalls/SystemCall.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tty/backends/BochsDebugTTY.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tty/backends/KeyboardInput.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tty/backends/VGATTY.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Memory/PageTable.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ProgramLoader/ELF.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/HAL/hal.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/HAL/IRQ.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/HAL/time.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/entry.cpp
    )

    set(kernel_lib_sources
        ${kernel_lib_sources}
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/src/mutex.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/src/semaphore.cpp
    )
endif()

if (FROSTYOS_BUILD_TARGET STREQUAL "Userland")
    set(kernel_sources
        ${kernel_sources}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/HAL/Userhal.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/HAL/UserPanic.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/HAL/UserTime.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Memory/tests/TestPageManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Memory/tests/TestPageTable.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Memory/tests/TestPPFA.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Memory/UserPageManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Memory/VirtualPageManager.cpp # this should be removed once testing is complete
        ${CMAKE_CURRENT_SOURCE_DIR}/src/UserEntry.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tty/backends/UserTTY.cpp
    )
endif()

set(kernel_sources
    ${kernel_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics/Colour.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Data-structures/AVLTree.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Data-structures/Bitmap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Data-structures/Buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Data-structures/LinkedList.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fs/DirectoryStream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fs/FileDescriptor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fs/FileDescriptorManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fs/FileStream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fs/initramfs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fs/TempFS/TempFileSystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fs/TempFS/TempFSInode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fs/VFS.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/kernel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Memory/kmalloc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Memory/Memory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Memory/newdelete.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Memory/PagingUtil.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Memory/PageObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Memory/VirtualRegion.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Memory/PageManagerCommon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sanitiser/sanitiser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sanitiser/ubsan.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sanitiser/asan.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tty/TTY.cpp
)

set(kernel_lib_sources
    ${kernel_lib_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/src/assert.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/src/icxxabi.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/src/math.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/src/stack_protector.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/src/stdio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/src/stdlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/src/string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/src/time.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/src/util.c
)

add_executable(kernel ${kernel_sources} ${kernel_lib_sources})

set_target_properties(kernel PROPERTIES CXX_STANDARD 23)
set_target_properties(kernel PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(kernel PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(kernel PROPERTIES C_STANDARD 23)
set_target_properties(kernel PROPERTIES C_EXTENSIONS OFF)
set_target_properties(kernel PROPERTIES C_STANDARD_REQUIRED ON)

target_include_directories(kernel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(kernel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/headers)
target_include_directories(kernel PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/include)

if (FROSTYOS_BUILD_TARGET STREQUAL "kernel")
    set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/kernel.ld)

    target_link_options(kernel
        PUBLIC -T
        PUBLIC ${LINKER_SCRIPT}
        PUBLIC -static
        PUBLIC -Bsymbolic
        PUBLIC -nostdlib
        PUBLIC -zmax-page-size=0x1000
        PRIVATE -mno-red-zone
    )

    target_compile_definitions(kernel PRIVATE _FROSTYOS_BUILD_TARGET=kernel)

    if (FROSTYOS_ENABLE_KASAN STREQUAL "yes")
        target_compile_definitions(kernel PRIVATE _FROSTYOS_ENABLE_KASAN=1)

        target_compile_options(kernel
            PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fsanitize=kernel-address,undefined>
        )
    else()
        target_compile_definitions(kernel PRIVATE _FROSTYOS_ENABLE_KASAN=0)
        
        target_compile_options(kernel
            PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fsanitize=undefined>
        )
    endif()

    target_compile_options(kernel
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-mcmodel=kernel>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-g>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wall>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wextra>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wpedantic>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wno-strict-aliasing>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wno-pointer-arith>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wno-unused-parameter>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wno-switch>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wno-packed-bitfield-compat>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-nostdlib>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-ffreestanding>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fno-pie>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fstack-protector>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-mgeneral-regs-only>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-mno-red-zone>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-O2>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fno-omit-frame-pointer>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    )

    target_include_directories(kernel PRIVATE ${UACPI_INCLUDES})

    target_link_libraries(kernel PRIVATE uACPI)
endif()

if (FROSTYOS_BUILD_TARGET STREQUAL "Userland")
    set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/user-kernel.ld)


    target_link_options(kernel
        PUBLIC -T
        PUBLIC ${LINKER_SCRIPT}
        PUBLIC -static
        PUBLIC -nostdlib
        PRIVATE -ffreestanding
        PUBLIC -zmax-page-size=0x1000
        PRIVATE -mno-red-zone
    )

    target_compile_definitions(kernel PRIVATE _FROSTYOS_BUILD_TARGET=Userland)

    target_compile_options(kernel
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-g>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wall>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wextra>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wpedantic>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wno-strict-aliasing>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wno-pointer-arith>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wno-unused-parameter>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wno-switch>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wno-packed-bitfield-compat>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-nostdlib>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-ffreestanding>
        #PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fPIE>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fsanitize=undefined,kernel-address>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fstack-protector>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-mgeneral-regs-only>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-mno-red-zone>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-mno-fsgsbase>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-mno-tls-direct-seg-refs>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-mstack-protector-guard=global>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-O2>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fno-omit-frame-pointer>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
        PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    )
endif()

set_target_properties(kernel PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})

target_link_libraries(kernel PRIVATE gcc)

target_compile_definitions(kernel PRIVATE _IS_IN_KERNEL=1)

if (FROSTYOS_BUILD_TARGET STREQUAL "kernel")

    add_custom_target(install_kernel_headers ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${FROSTYOS_INSTALL_PREFIX}/include/kernel
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/headers ${FROSTYOS_INSTALL_PREFIX}/include/kernel
    )

    add_custom_target(create_dist_dir ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/dist/boot/FrostyOS
    )

    add_dependencies(kernel limine_include install_kernel_headers create_dist_dir)

    add_custom_target(install_kernel
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/dist/boot/FrostyOS
        # COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:kernel> ${CMAKE_SOURCE_DIR}/dist/boot/FrostyOS/kernel.elf
        COMMAND ${CMAKE_NM} -C --format=bsd -n $<TARGET_FILE:kernel> | ${CMAKE_SOURCE_DIR}/utils/bin/buildsymboltable ${CMAKE_SOURCE_DIR}/root/kernel.map
        COMMAND ${CMAKE_STRIP} --strip-debug $<TARGET_FILE:kernel> -o ${CMAKE_SOURCE_DIR}/dist/boot/FrostyOS/kernel.elf
    )

endif()