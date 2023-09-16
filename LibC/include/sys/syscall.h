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

#ifndef _SYS_SYSCALL_H
#define _SYS_SYSCALL_H

#ifdef __cplusplus
extern "C" {
#endif

enum SystemCalls {
    SC_EXIT = 0
};

unsigned long system_call(unsigned long num, unsigned long arg1, unsigned long arg2, unsigned arg3);


#ifdef __cplusplus
}
#endif

#endif /* _SYS_SYSCALL_H */