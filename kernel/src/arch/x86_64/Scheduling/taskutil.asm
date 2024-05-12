; Copyright (©) 2023-2024  Frosty515
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

extern kernel_stack
extern kernel_stack_size

global x86_64_PrepareThreadExit
x86_64_PrepareThreadExit:
    mov rsp, kernel_stack
    add rsp, [kernel_stack_size]
    xor rbp, rbp
    call rcx
    cli
.l: ; we should never get to here. just hang.
    hlt
    jmp .l

global x86_64_GetReturnAddress
x86_64_GetReturnAddress:
    mov rax, QWORD [rbp+8]
    ret

extern x86_64_HandleSemaphoreAcquire

global x86_64_Prep_SemaphoreAcquire
x86_64_Prep_SemaphoreAcquire:
    pushf
    cli
    push rax
    push rbp
    mov rbp, rsp

    sub rsp, 4 ; used to fix alignment later
    mov rax, cr3
    push rax
    push QWORD [rbp+16]

    sub rsp, 2
    mov WORD [rsp], 0x10 ; DS
    sub rsp, 2
    mov WORD [rsp], 0x08 ; CS

    push 0 ; deal with rip later

    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push QWORD [rbp]
    push 0 ; deal with rsp later
    push rdi
    push rsi
    push 0 ; rdx must be 0 for this to work
    push rcx
    push rbx
    push QWORD [rbp+8]

    ; everything is saved, now we sort out rsp and rip

    ; rsp
    lea rax, QWORD [rbp+24] ; make it look like this function wasn't called
    mov QWORD [rsp+48], rax

    ; RIP
    mov QWORD [rsp+128], x86_64_HandleSemaphoreAcquire

    mov rdx, rsp
    jmp x86_64_HandleSemaphoreAcquire
