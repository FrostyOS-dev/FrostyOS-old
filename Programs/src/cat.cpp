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

int main(int argc, char** argv) {
    if (argc <= 1) {
        int c;
        while ((c = getc()) != EOF)
            putc(c);
        return 0;
    }
    for (int i = 1; i < argc; i++) {
        if (0 == strcmp(argv[i], "-")) {
            int c;
            while ((c = getc()) != EOF)
                putc(c);
        }
        else {
            FILE* f = fopen(argv[i], "r");
            if (!f)
                return 1;
            int c;
            while ((c = fgetc(f)) != EOF)
                putc(c);
            fclose(f);
        }
    }
    return 0;
}
