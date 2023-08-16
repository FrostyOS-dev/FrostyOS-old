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

global CombineDWords
CombineDWords:
    xor rax, rax
    mov eax, edi
    shl rax, 32
    mov eax, esi
    ret

global CombineWords
CombineWords:
    xor rax, rax
    mov ax, di
    shl eax, 16
    mov ax, si
    ret

global CombineBytes
CombineBytes:
    xor rax, rax
    mov al, dil
    shl ax, 8
    mov al, sil
    ret
