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

if(WORLDOS_ARCHITECTURE STREQUAL "x86_64")
    set(libc_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/inttypes.asm
        ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/stdlib.asm
        ${CMAKE_CURRENT_SOURCE_DIR}/src/arch/x86_64/string.asm
    )
endif()

set(libc_sources
    ${libc_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/assert.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ctype.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/errno.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/init.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/inttypes.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/malloc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/signal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stdio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stdlib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/string.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/util.c
    ${CMAKE_CURRENT_SOURCE_DIR}/ssp/src/stack_protector.c
)

add_library(c STATIC ${libc_sources})

target_include_directories(c PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(c PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_options(c 
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wall>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wextra>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wpedantic>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wstrict-aliasing>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wunused-parameter>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-nostdlib>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-ffreestanding>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fstack-protector>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fno-omit-frame-pointer>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-mgeneral-regs-only>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-mno-red-zone>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-O2>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
)

set_target_properties(c PROPERTIES CXX_STANDARD 23)
set_target_properties(c PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(c PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(c PROPERTIES C_STANDARD 23)
set_target_properties(c PROPERTIES C_EXTENSIONS OFF)
set_target_properties(c PROPERTIES C_STANDARD_REQUIRED ON)

add_custom_target(install_c_headers
    COMMAND ${CMAKE_COMMAND} -E make_directory ${WORLDOS_INSTALL_PREFIX}/include
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/include ${WORLDOS_INSTALL_PREFIX}/include
)

add_custom_target(install_libc
    COMMAND ${CMAKE_COMMAND} -E make_directory ${WORLDOS_INSTALL_PREFIX}/lib
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:c> ${WORLDOS_INSTALL_PREFIX}/lib
)

add_dependencies(c kernel install_c_headers create_fake_libraries)

add_dependencies(install_libc c install_ssp install_startfiles)

# libssp

set(libssp_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/ssp/src/stack_protector.c
)

add_library(ssp STATIC ${libssp_sources})

target_include_directories(ssp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(ssp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_options(ssp 
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wall>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wextra>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wpedantic>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wstrict-aliasing>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wunused-parameter>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-nostdlib>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-ffreestanding>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fstack-protector>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fno-omit-frame-pointer>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-mgeneral-regs-only>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-mno-red-zone>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-O2>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
)

set_target_properties(ssp PROPERTIES CXX_STANDARD 23)
set_target_properties(ssp PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(ssp PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(ssp PROPERTIES C_STANDARD 23)
set_target_properties(ssp PROPERTIES C_EXTENSIONS OFF)
set_target_properties(ssp PROPERTIES C_STANDARD_REQUIRED ON)

add_dependencies(ssp kernel)

add_custom_target(install_ssp
    COMMAND ${CMAKE_COMMAND} -E make_directory ${WORLDOS_INSTALL_PREFIX}/lib
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ssp> ${WORLDOS_INSTALL_PREFIX}/lib
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${WORLDOS_INSTALL_PREFIX}/lib/libssp.a ${WORLDOS_INSTALL_PREFIX}/lib/libssp_nonshared.a
)


# startfiles

add_custom_target(install_startfiles)

if(WORLDOS_ARCHITECTURE STREQUAL "x86_64")
    set(startfiles_sources
        ${startfiles_sources}
        ${CMAKE_CURRENT_SOURCE_DIR}/startfiles/src/arch/x86_64/crt0.asm
        ${CMAKE_CURRENT_SOURCE_DIR}/startfiles/src/arch/x86_64/crti.asm
        ${CMAKE_CURRENT_SOURCE_DIR}/startfiles/src/arch/x86_64/crtn.asm
    )
    add_library(crt0 OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/startfiles/src/arch/x86_64/crt0.asm)
    add_library(crti OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/startfiles/src/arch/x86_64/crti.asm)
    add_library(crtn OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/startfiles/src/arch/x86_64/crtn.asm)

    add_custom_target(install_crt0
        COMMAND ${CMAKE_COMMAND} -E make_directory ${WORLDOS_INSTALL_PREFIX}/lib
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:crt0> ${WORLDOS_INSTALL_PREFIX}/lib
        COMMAND ${CMAKE_COMMAND} -E rename ${WORLDOS_INSTALL_PREFIX}/lib/crt0.asm.o ${WORLDOS_INSTALL_PREFIX}/lib/crt0.o
    )

    add_dependencies(install_crt0 crt0)

    add_custom_target(install_crti
        COMMAND ${CMAKE_COMMAND} -E make_directory ${WORLDOS_INSTALL_PREFIX}/lib
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:crti> ${WORLDOS_INSTALL_PREFIX}/lib
        COMMAND ${CMAKE_COMMAND} -E rename ${WORLDOS_INSTALL_PREFIX}/lib/crti.asm.o ${WORLDOS_INSTALL_PREFIX}/lib/crti.o
    )

    add_dependencies(install_crti crti)

    add_custom_target(install_crtn
        COMMAND ${CMAKE_COMMAND} -E make_directory ${WORLDOS_INSTALL_PREFIX}/lib
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:crtn> ${WORLDOS_INSTALL_PREFIX}/lib
        COMMAND ${CMAKE_COMMAND} -E rename ${WORLDOS_INSTALL_PREFIX}/lib/crtn.asm.o ${WORLDOS_INSTALL_PREFIX}/lib/crtn.o
    )

    add_dependencies(install_crtn crtn)

    add_dependencies(install_startfiles install_crt0 install_crti install_crtn)
endif()
