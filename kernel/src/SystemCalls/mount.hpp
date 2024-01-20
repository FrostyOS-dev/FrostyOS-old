/*
Copyright (©) 2024  Frosty515

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

#ifndef _MOUNT_HPP
#define _MOUNT_HPP

#include <fs/FileSystem.hpp>

#include <Scheduling/Thread.hpp>

int Mount(const char* source, const char* target, FileSystemType type, FilePrivilegeLevel current_privilege = {0, 0, 00755});
int Unmount(const char* target, FilePrivilegeLevel current_privilege = {0, 0, 00755});

int sys$mount(Scheduling::Thread* thread, const char* target, const char* type, const char* device);
int sys$unmount(Scheduling::Thread* thread, const char* target);

#endif /* _MOUNT_HPP */