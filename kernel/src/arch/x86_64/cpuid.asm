; Copyright (©) 2022-2023  Frosty515
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

global x86_64_cpuid
x86_64_cpuid:
    push rbp
    mov rbp, rsp

    push rbx ; save rbx
    xor rax, rax
    xor rbx, rbx
    xor rcx, rcx
    xor rdx, rdx
    mov eax, edi ; get lower 32-bits of rdi
    shr rdi, 32
    mov ebx, edi ; get upper 32-bits of rdi
    mov ecx, esi ; get lower 32-bits of rsi
    shr rsi, 32
    mov edx, esi ; get upper 32-bits of rsi
    cli
    cpuid
    shl rbx, 32
    or rax, rbx ; mov ebx into upper 32-bits of rax
    shl rdx, 32 ; mov edx into upper 32-bits of rdx
    or rdx, rcx ; mov ecx into lower 32-bits of rdx
    pop rbx ; restore rbx

    mov rsp, rbp
    pop rbp
    ret
