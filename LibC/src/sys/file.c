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

#include <sys/file.h>
#include <sys/syscall.h>

fd_t open(const char* path, unsigned long mode) {
    return (fd_t)system_call(SC_OPEN, (unsigned long)path, mode, 0);
}

long read(fd_t file, void* buf, unsigned long count) {
    return (long)system_call(SC_READ, (unsigned long)file, (unsigned long)buf, count);
}

long write(fd_t file, const void* buf, unsigned long count) {
    return (long)system_call(SC_WRITE, (unsigned long)file, (unsigned long)buf, count);
}

int close(fd_t file) {
    return (int)system_call(SC_CLOSE, (unsigned long)file, 0, 0);
}

long seek(fd_t file, long offset) {
    return (long)system_call(SC_SEEK, (unsigned long)file, (unsigned long)offset, 0);
}