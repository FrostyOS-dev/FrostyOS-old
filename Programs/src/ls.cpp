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

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <kernel/file.h>

uint8_t GetSize(uint64_t size, char* c) {
    uint64_t i_size = 0;
    if (size < 1024) {
        if (c != nullptr)
            *c = ' ';
        i_size = size;
    }
    else if ((size & ~((1UL << 10) - 1)) > 0) {
        if (c != nullptr)
            *c = 'K';
        i_size = size >> 10;
    }
    else if ((size & ~((1UL << 20) - 1)) > 0) {
        if (c != nullptr)
            *c = 'M';
        i_size = size >> 20;
    }
    else if ((size & ~((1UL << 30) - 1)) > 0) {
        if (c != nullptr)
            *c = 'G';
        i_size = size >> 30;
    }
    else if ((size & ~((1UL << 40) - 1)) > 0) {
        if (c != nullptr)
            *c = 'T';
        i_size = size >> 40;
    }
    else if ((size & ~((1UL << 50) - 1)) > 0) {
        if (c != nullptr)
            *c = 'P';
        i_size = size >> 50;
    }
    else {
        if (c != nullptr)
            *c = 'E';
        i_size = size >> 60;
    }
    return (uint8_t)i_size;
}

// Argument layout: [path]

int main(int argc, char** argv) {
    char const* path = "";
    if (argc == 1)
        path = "/";
    else if (argc == 2) {
        char const* i_path = argv[1];
        struct stat_buf buf;
        int ret = stat(i_path, &buf);
        if (ret < 0) {
            perror("stat");
            return 1;
        }

        if (buf.st_type == DT_FILE) {
            char const* name = nullptr;
            char* rc = strrchr(i_path, '/');
            if (rc == nullptr) // must be in the root directory
                name = i_path;
            else
                name = (char const*)((uint64_t)rc + sizeof(char));
            char c = 0;
            uint8_t size = GetSize(buf.st_size, &c);
            printf("-%c%c%c%c%c%c%c%c%c %u %u %3hhu%cB %s\n",
                (buf.st_mode & 0x100) > 0 ? 'r' : '-',
                (buf.st_mode & 0x80) > 0 ? 'w' : '-',
                (buf.st_mode & 0x40) > 0 ? 'x' : '-',
                (buf.st_mode & 0x20) > 0 ? 'r' : '-',
                (buf.st_mode & 0x10) > 0 ? 'w' : '-',
                (buf.st_mode & 0x8) > 0 ? 'x' : '-',
                (buf.st_mode & 0x4) > 0 ? 'r' : '-',
                (buf.st_mode & 0x2) > 0 ? 'w' : '-',
                (buf.st_mode & 0x1) > 0 ? 'x' : '-', buf.st_uid, buf.st_gid, size, c, name);
            return 0;
        }
        else if (buf.st_type == DT_DIR)
            path = i_path;
        else {
            printf("%s: path type is unknown\n", argv[0]);
            return 1;
        }
    }
    else {
        printf("Usage: %s [path]\n", argv[0]);
        return 1;
    }
    fd_t fd = open(path, O_READ, 0);
    if (fd < 0) {
        fprintf(stderr, "open: %s", strerror(-fd));
        return 1;
    }
    while (true) {
        struct dirent dirents[1];
        int ret = getdirents(fd, dirents, 1);
        if (ret < 0)
            break;
        struct dirent dir = dirents[0];
        char type = 0;
        switch (dir.d_type) {
        case DT_FILE:
            type = '-';
            break;
        case DT_DIR:
            type = 'd';
            break;
        case DT_SYMLNK:
            type = 'l';
            break;
        default:
            type = '?';
            break;
        }
        char c = 0;
        uint8_t size = GetSize(dir.d_size, &c);
        printf("%c%c%c%c%c%c%c%c%c%c %u %u %3hhu%cB %s\n", type,
            (dir.d_mode & 0x100) > 0 ? 'r' : '-',
            (dir.d_mode & 0x80) > 0 ? 'w' : '-',
            (dir.d_mode & 0x40) > 0 ? 'x' : '-',
            (dir.d_mode & 0x20) > 0 ? 'r' : '-',
            (dir.d_mode & 0x10) > 0 ? 'w' : '-',
            (dir.d_mode & 0x8) > 0 ? 'x' : '-',
            (dir.d_mode & 0x4) > 0 ? 'r' : '-',
            (dir.d_mode & 0x2) > 0 ? 'w' : '-',
            (dir.d_mode & 0x1) > 0 ? 'x' : '-', dir.d_uid, dir.d_gid, size, c, dir.d_name);
    }
    close(fd);
    return 0;
}