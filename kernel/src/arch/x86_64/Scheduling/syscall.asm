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

extern kernel_stack
extern kernel_stack_size

global x86_64_IsSystemCallSupported
x86_64_IsSystemCallSupported:
    push rbp
    mov rbp, rsp

    cli

    push rbx
    xor rax, rax
    xor rbx, rbx
    xor rcx, rcx
    xor rdx, rdx

    mov eax, 0x80000001 ; Extended Function CPUID Information
    cpuid
    and rdx, 0x800 ; bit 11
    shr rdx, 11
    mov rax, rdx

    pop rbx

    sti

    mov rsp, rbp
    pop rbp
    ret

global x86_64_EnableSystemCalls
x86_64_EnableSystemCalls:
    push rbp
    mov rbp, rsp
    push rdx

    call x86_64_IsSystemCallSupported
    test rax, rax
    jz .fail

    ; enable system calls
    xor rax, rax
    xor rdx, rdx
    mov rcx, 0xc0000080 ; EFER
    rdmsr
    or rax, 1
    wrmsr

    ; set the segments
    xor rax, rax
    sub si, 0x8
    movzx rdx, si
    shl edx, 16
    movzx edi, di
    or edx, edi
    mov rcx, 0xc0000081 ; STAR
    wrmsr

    ; set the handler address
    xor rax, rax
    pop rdx
    mov eax, edx
    shr rdx, 32
    mov rcx, 0xc0000082 ; LSTAR
    wrmsr

    mov rax, 0x200 ; Interrupt flag
    xor rdx, rdx
    mov rcx, 0xc0000084 ; FMASK
    wrmsr

    ; clear unused register

    xor rax, rax
    xor rdx, rdx
    mov rcx, 0xc0000083 ; CSTAR
    wrmsr

.success:
    mov rax, 1
    jmp .end

.fail:
    xor rax, rax

.end:
    mov rsp, rbp
    pop rbp
    ret

extern SystemCallHandler

global x86_64_HandleSystemCall
x86_64_HandleSystemCall:
    swapgs ; get kernel gs base

    mov QWORD [gs:0], rsp ; save user stack
    mov rsp, QWORD [gs:8] ; get kernel stack

    sub rsp, 4 ; used to fix alignment later
    push 0 ; dummy cr3 value until we can actually save it
    push r11 ; rflags

    sub rsp, 2
    mov WORD [rsp], 0x1b ; DS
    sub rsp, 2
    mov WORD [rsp], 0x23 ; CS

    push rcx ; rip
    push r15
    push r14

    ; we now have at least 2 free GPRs, so we can save the kernel stack better and save the user stack in the register frame.
    lea r14, QWORD [rsp+36]
    mov r15, QWORD [gs:0]

    ; don't need gs base anymore, so we can restore it
    swapgs

    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push r15 ; user stack
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push rax

    mov rax, cr3
    mov QWORD [r14], rax ; save cr3

    mov ax, 0x10 ; load segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    sti ; safe to enable interrupts

    mov rcx, rdx ; arg3
    mov rdx, rsi ; arg2
    mov rsi, rdi ; arg1
    mov rdi, QWORD [rsp] ; restore the syscall number
    mov r8, rsp ; save address

    ;xor rbp, rbp ; create a blank stack frame

    call SystemCallHandler

    cli ; interrupts must be disabled again

    mov QWORD [rsp], rax ; save rax

    add rsp, 8 ; skip restoring rax
    pop rbx
    add rsp, 8 ; don't restore rcx twice
    pop rdx
    pop rsi
    pop rdi
    add rsp, 8 ; skip restore user stack for now
    pop rbp
    pop r8
    pop r9
    pop r10
    add rsp, 8 ; don't restore r11 twice
    pop r12
    pop r13
    pop r14
    pop r15
    pop rcx ; return address
    add rsp, 4 ; don't need to restore cs and ds as they are known values
    pop r11 ; rflags
    pop rax
    mov cr3, rax

    add rsp, 4 ; restore alignment changes

    mov ax, 0x1b
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov rax, QWORD [rsp-160] ; get return value

    swapgs ; get offset again

    mov rsp, QWORD [gs:0]

    swapgs ; restore gs base

    o64 sysret
