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

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: chown <file> <uid>[:<gid>]\n");
        return 1;
    }

    unsigned int uid = 0;
    unsigned int gid = (unsigned int)-1;
    char* colon = strchr(argv[2], ':');
    if (colon != nullptr) {
        *colon = 0;
        gid = atoi(colon + 1);
    }
    uid = atoi(argv[2]);

    if (chown(argv[1], uid, gid) < 0) {
        printf("Failed to chown. errno = %d\n", errno);
        return 1;
    }

    return 0;
}