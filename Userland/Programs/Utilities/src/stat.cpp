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
#include <errno.h>

// Argument layout: <file>
int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: stat <file>\n");
        return 1;
    }

    struct stat_buf buf = {0, 0, 0, 0, 0};
    if (stat(argv[1], &buf) < 0) {
        perror("stat");
        return 1;
    }

    printf("Size: %lu\n", buf.st_size);
    printf("Mode: %04ho\n", buf.st_mode);
    printf("UID: %u\n", buf.st_uid);
    printf("GID: %u\n", buf.st_gid);
    printf("Type: %s\n", buf.st_type == DT_FILE ? "file" : (buf.st_type == DT_DIR ? "directory" : (buf.st_type == DT_SYMLNK ? "symlink" : "unknown")));

    return 0;
}