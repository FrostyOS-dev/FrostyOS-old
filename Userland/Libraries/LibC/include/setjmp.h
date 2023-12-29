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

#ifndef _SETJMP_H
#define _SETJMP_H

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __x86_64__
typedef struct {
    unsigned long int r15;
    unsigned long int r14;
    unsigned long int r13;
    unsigned long int r12;
    unsigned long int r11;
    unsigned long int r10;
    unsigned long int r9;
    unsigned long int r8;

    unsigned long int rbp;
    unsigned long int rsp;

    unsigned long int rdi;
    unsigned long int rsi;
    unsigned long int rdx;
    unsigned long int rcx;
    unsigned long int rbx;
    unsigned long int rax;

    unsigned long int rip;
    unsigned long int rflags;
    unsigned short int cs;
    unsigned short int ss;
} jmp_buf;

int setjmp(jmp_buf env);

void longjmp(jmp_buf env, int val);

#endif /* __x86_64__ */


#ifdef __cplusplus
}
#endif

#endif /* _SETJMP_H */