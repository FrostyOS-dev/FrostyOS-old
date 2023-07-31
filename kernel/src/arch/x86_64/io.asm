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

global x86_64_outb
x86_64_outb:
    mov al, sil ; value must be in al or the instruction won't work
    mov dx, di ; port must be in dx or instruction won't work
    out dx, al
    ret

global x86_64_inb
x86_64_inb:
    mov dx, di ; port must be in dx
    xor rax, rax ; clear rax
    in al, dx
    ret

global x86_64_EnableInterrupts
x86_64_EnableInterrupts:
    sti
    ret

global x86_64_DisableInterrupts
x86_64_DisableInterrupts:
    cli
    ret

global x86_64_iowait
x86_64_iowait:
    xor rax, rax ; clear rax
    out 0x80, al ; 0x80 is an unused port
    ret
