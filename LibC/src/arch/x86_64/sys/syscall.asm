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

global system_call
system_call:
    push rbp
    mov rbp, rsp
    push r11 ; gets overridden by syscall
    mov rax, rdi ; set the system call number
    mov rdi, rsi ; set arg1
    mov rsi, rdx ; set arg2
    mov rdx, rcx ; set arg2
    syscall
    pop r11
    mov rsp, rbp
    pop rbp
    ret
