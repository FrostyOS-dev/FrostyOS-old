; Copyright (©) 2023  Frosty515
; 
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
; 
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
; 
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <https://www.gnu.org/licenses/>.

[bits 64]

extern __init_libc
extern _init
extern main
extern exit

global _start
_start:
    xor rbp, rbp
    push rbp ; rip
    push rbp ; rbp
    mov rbp, rsp

    mov rax, rdi
    mov rdi, QWORD [rax]
    mov rsi, QWORD [rax+8]
    mov rdx, QWORD [rax+16]
    mov rcx, QWORD [rax+24]

    call __init_libc
    call _init

    call main

    mov rdi, rax
    call exit
