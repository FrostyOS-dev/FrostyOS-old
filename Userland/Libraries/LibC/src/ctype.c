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

#include "ctype.h"

int isalnum(int c) {
    if (isalpha(c) || isdigit(c))
        return 1;
    return 0;
}

int isalpha(int c) {
    if (islower(c) || isupper(c))
        return 1;
    return 0;
}

int isblank(int c) {
    if (c == ' ' || c == '\t')
        return 1;
    return 0;
}

int iscntrl(int c) {
    if (c < 32 || c == 127)
        return 1;
    return 0;
}

int isdigit(int c) {
    if (c >= '0' && c <= '9')
        return 1;
    return 0;
}

int isgraph(int c) {
    if (c >= 33 && c <= 126)
        return 1;
    return 0;
}

int islower(int c) {
    if (c >= 'a' && c <= 'z')
        return 1;
    return 0;
}

int isprint(int c) {
    if (c >= 32 && c <= 126)
        return 1;
    return 0;
}

int ispunct(int c) {
    if (isprint(c) && !isalnum(c) && !isspace(c))
        return 1;
    return 0;
}

int isspace(int c) {
    if (c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r')
        return 1;
    return 0;
}

int isupper(int c) {
    if (c >= 'A' && c <= 'Z')
        return 1;
    return 0;
}

int isxdigit(int c) {
    if (isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
        return 1;
    return 0;
}

int tolower(int c) {
    if (isupper(c))
        return c - 'A' + 'a';
    return c;
}

int toupper(int c) {
    if (islower(c))
        return c - 'a' + 'A';
    return c;
}