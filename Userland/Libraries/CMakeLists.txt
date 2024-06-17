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

# we need to create empty libm.a and libstdc++.a libraries in ${FROSTYOS_INSTALL_PREFIX}/lib/ as libm is actually inside LibC and libstdc++ doesn't exist yet
add_custom_target(create_fake_libraries
    COMMAND ${CMAKE_COMMAND} -E make_directory ${FROSTYOS_INSTALL_PREFIX}/lib
    COMMAND ${CMAKE_COMMAND} -E touch ${FROSTYOS_INSTALL_PREFIX}/lib/libm.a
    COMMAND ${CMAKE_COMMAND} -E touch ${FROSTYOS_INSTALL_PREFIX}/lib/libstdc++.a
)

add_custom_target(Libraries)

add_dependencies(Libraries create_fake_libraries install_libc)

add_subdirectory(LibC)