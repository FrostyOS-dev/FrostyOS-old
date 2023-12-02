/*
Copyright (©) 2023  Frosty515

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _FILE_PRIVILEGE_LEVEL_HPP
#define _FILE_PRIVILEGE_LEVEL_HPP

#include <stdint.h>

#define ACL_USER_READ 00400
#define ACL_USER_WRITE 00200
#define ACL_USER_EXECUTE 00100
#define ACL_GROUP_READ 00040
#define ACL_GROUP_WRITE 00020
#define ACL_GROUP_EXECUTE 00010
#define ACL_OTHER_READ 00004
#define ACL_OTHER_WRITE 00002
#define ACL_OTHER_EXECUTE 00001

struct FilePrivilegeLevel {
    uint32_t UID;
    uint32_t GID;
    uint16_t ACL;
};

#endif /* _FILE_PRIVILEGE_LEVEL_HPP */