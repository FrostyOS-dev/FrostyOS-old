; Copyright (©) 2024  Frosty515
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

global x86_64_ReadMSR
x86_64_ReadMSR:
    xor rax, rax
    xor rcx, rcx
    xor rdx, rdx

    mov ecx, edi
    rdmsr
    shl rdx, 32
    or rax, rdx
    ret

global x86_64_WriteMSR
x86_64_WriteMSR:
    xor rax, rax
    xor rcx, rcx
    mov ecx, edi

    mov eax, esi
    mov rdx, rsi
    shr rdx, 32
    wrmsr
    ret
