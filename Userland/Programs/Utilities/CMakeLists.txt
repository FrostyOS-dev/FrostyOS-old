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

add_executable(cat src/cat.cpp)
add_executable(chmod src/chmod.cpp)
add_executable(chown src/chown.cpp)
add_executable(echo src/echo.cpp)
add_executable(ls src/ls.cpp)
add_executable(stat src/stat.cpp)


target_compile_options(cat
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wall>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wextra>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fstack-protector>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fno-omit-frame-pointer>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-mgeneral-regs-only>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-O2>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
)

set_target_properties(cat PROPERTIES CXX_STANDARD 23)
set_target_properties(cat PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(cat PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(cat PROPERTIES C_STANDARD 23)
set_target_properties(cat PROPERTIES C_EXTENSIONS OFF)
set_target_properties(cat PROPERTIES C_STANDARD_REQUIRED ON)

add_dependencies(cat install_libc)


target_compile_options(chmod
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wall>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wextra>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fstack-protector>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fno-omit-frame-pointer>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-mgeneral-regs-only>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-O2>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
)

set_target_properties(chmod PROPERTIES CXX_STANDARD 23)
set_target_properties(chmod PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(chmod PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(chmod PROPERTIES C_STANDARD 23)
set_target_properties(chmod PROPERTIES C_EXTENSIONS OFF)
set_target_properties(chmod PROPERTIES C_STANDARD_REQUIRED ON)

add_dependencies(chmod install_libc)


target_compile_options(chown
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wall>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wextra>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fstack-protector>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fno-omit-frame-pointer>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-mgeneral-regs-only>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-O2>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
)

set_target_properties(chown PROPERTIES CXX_STANDARD 23)
set_target_properties(chown PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(chown PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(chown PROPERTIES C_STANDARD 23)
set_target_properties(chown PROPERTIES C_EXTENSIONS OFF)
set_target_properties(chown PROPERTIES C_STANDARD_REQUIRED ON)

add_dependencies(chown install_libc)


target_compile_options(echo
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wall>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wextra>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fstack-protector>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fno-omit-frame-pointer>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-mgeneral-regs-only>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-O2>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
)

set_target_properties(echo PROPERTIES CXX_STANDARD 23)
set_target_properties(echo PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(echo PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(echo PROPERTIES C_STANDARD 23)
set_target_properties(echo PROPERTIES C_EXTENSIONS OFF)
set_target_properties(echo PROPERTIES C_STANDARD_REQUIRED ON)

add_dependencies(echo install_libc)


target_compile_options(ls
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wall>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wextra>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fstack-protector>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fno-omit-frame-pointer>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-mgeneral-regs-only>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-O2>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
)

set_target_properties(ls PROPERTIES CXX_STANDARD 23)
set_target_properties(ls PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(ls PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(ls PROPERTIES C_STANDARD 23)
set_target_properties(ls PROPERTIES C_EXTENSIONS OFF)
set_target_properties(ls PROPERTIES C_STANDARD_REQUIRED ON)

add_dependencies(ls install_libc)


target_compile_options(stat
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wall>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-Wextra>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fstack-protector>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-fno-omit-frame-pointer>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-mgeneral-regs-only>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX,C>:-O2>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
)

set_target_properties(stat PROPERTIES CXX_STANDARD 23)
set_target_properties(stat PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(stat PROPERTIES CXX_EXTENSIONS OFF)
set_target_properties(stat PROPERTIES C_STANDARD 23)
set_target_properties(stat PROPERTIES C_EXTENSIONS OFF)
set_target_properties(stat PROPERTIES C_STANDARD_REQUIRED ON)

add_dependencies(stat install_libc)

add_custom_target(Utilities
    COMMAND ${CMAKE_COMMAND} -E make_directory ${FROSTYOS_INSTALL_PREFIX}/bin
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:cat> ${FROSTYOS_INSTALL_PREFIX}/bin/cat
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:chmod> ${FROSTYOS_INSTALL_PREFIX}/bin/chmod
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:chown> ${FROSTYOS_INSTALL_PREFIX}/bin/chown
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:echo> ${FROSTYOS_INSTALL_PREFIX}/bin/echo
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ls> ${FROSTYOS_INSTALL_PREFIX}/bin/ls
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:stat> ${FROSTYOS_INSTALL_PREFIX}/bin/stat
)

add_dependencies(Utilities cat chmod chown echo ls stat)