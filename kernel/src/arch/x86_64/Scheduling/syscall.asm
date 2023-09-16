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

    ; clear unused registers

    xor rax, rax
    xor rdx, rdx
    mov rcx, 0xc0000083 ; CSTAR
    wrmsr
    inc rcx
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
    cli
    swapgs ; get the offset to register save point

    mov QWORD [gs:48], rsp ; save user stack
    mov rsp, QWORD [gs:156] ; load kernel stack

    mov QWORD [gs:0], rax    ; save rax
    mov QWORD [gs:8], rbx    ; save rbx
    mov QWORD [gs:16], rcx   ; save rcx
    mov QWORD [gs:24], rdx   ; save rdx
    mov QWORD [gs:32], rsi   ; save rsi
    mov QWORD [gs:40], rdi   ; save rdi
    mov QWORD [gs:56], rbp   ; save rbp
    mov QWORD [gs:64],  r8   ; save r8
    mov QWORD [gs:72],  r9   ; save r9
    mov QWORD [gs:80], r10   ; save r10
    mov QWORD [gs:88], r11   ; save r11
    mov QWORD [gs:96], r12   ; save r12
    mov QWORD [gs:104], r13  ; save r13
    mov QWORD [gs:112], r14  ; save r14
    mov QWORD [gs:120], r15  ; save r15
    mov QWORD [gs:128], rcx  ; save rip
    mov  WORD [gs:136], 0x23 ; save cs
    mov  WORD [gs:138], 0x1b ; save ds
    mov QWORD [gs:140], r11  ; save rflags

    push rax ; preserve rax

    mov rax, cr3
    mov QWORD [gs:148], rax  ; save cr3

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax

    pop rax ; restore rax

    ; setup blank stack frame so we don't accidently read the user stack when a stack trace is performed
    xor rbp, rbp
    push rbp ; rbp

    mov rcx, rdx
    mov rdx, rsi
    mov rsi, rdi
    mov rdi, rax

    call SystemCallHandler

    add rsp, 8 ; discard useless rbp value

    push rax ; save return value

    mov ax, 0x1b
    mov ds, ax
    mov es, ax
    mov fs, ax

    mov QWORD rbx, [gs:8]    ; load rbx
    mov QWORD rcx, [gs:16]   ; load rcx and rip
    mov QWORD rdx, [gs:24]   ; load rdx
    mov QWORD rsi, [gs:32]   ; load rsi
    mov QWORD rdi, [gs:40]   ; load rdi
    mov QWORD rbp, [gs:56]   ; load rbp
    mov QWORD r8, [gs:64]   ; load r8
    mov QWORD r9, [gs:72]   ; load r9
    mov QWORD r10, [gs:80]   ; load r10
    mov QWORD r11, [gs:88]   ; load r11 and rflags
    mov QWORD r12, [gs:96]   ; load r12
    mov QWORD r13, [gs:104]  ; load r13
    mov QWORD r14, [gs:112]  ; load r14
    mov QWORD r15, [gs:120]  ; load r15

    mov rax, QWORD [gs:148]  ; load cr3
    mov cr3, rax

    pop rax ; restore return value

    mov QWORD [gs:156], rsp ; save kernel stack
    mov rsp, QWORD [gs:48] ; load user stack

    swapgs ; restore user GS base
    o64 sysret
