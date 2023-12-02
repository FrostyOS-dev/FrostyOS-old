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
#include <stdlib.h>
#include <errno.h>

// Argument layout: <file> <mode>

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: chmod <file> <mode>\n");
        return 1;
    }

    unsigned int mode = 0;
    char* endptr = nullptr;
    mode = strtoul(argv[2], &endptr, 8);
    if (endptr == argv[2]) {
        printf("Invalid mode\n");
        return 1;
    }

    if (chmod(argv[1], mode) < 0) {
        printf("Failed to chmod. errno = %d\n", errno);
        return 1;
    }

    return 0;
}
