[bits 64]

global x86_64_kernel_switch
x86_64_kernel_switch:
    cli ; disable interrupts

    mov rbx, QWORD [rdi+8]   ; load rbx
    mov rcx, QWORD [rdi+16]  ; load rcx
    mov rdx, QWORD [rdi+24]  ; load rdx
    mov rsi, QWORD [rdi+32]  ; load rsi
    mov rbp, QWORD [rdi+56]  ; load rbp
    mov  r8, QWORD [rdi+64]  ; load r8
    mov  r9, QWORD [rdi+72]  ; load r9
    mov r10, QWORD [rdi+80]  ; load r10
    mov r11, QWORD [rdi+88]  ; load r11
    mov r12, QWORD [rdi+96]  ; load r12
    mov r13, QWORD [rdi+104] ; load r13
    mov r14, QWORD [rdi+112] ; load r14
    mov r15, QWORD [rdi+120] ; load r15

    mov ax, WORD [rdi+138]
    mov ds, ax ; load ds
    mov es, ax ; load es
    mov fs, ax ; load fs
    mov gs, ax ; load gs
    mov ss, ax ; load ss

    mov rax, QWORD [rdi+148]
    mov cr3, rax ; load CR3

    mov rsp, QWORD [rdi+48] ; load rsp

    xor rax, rax
    mov ax, WORD [rdi+136]
    push rax ; prepare cs

    mov rax, QWORD [rdi+128]
    push rax ; prepare RIP

    mov rax, QWORD [rdi+140]
    push rax
    popf ; load RFLAGS
    
    mov rax, QWORD [rdi] ; load rax
    mov rdi, QWORD [rdi+40] ; load rdi

    retfq ; return to cs:RIP

global x86_64_kernel_save_main
x86_64_kernel_save_main:
    cli ; disable interrupts

    push rax ; prepare rax to be saved later
    mov rax, [rsp-24]

    mov QWORD [rax+8], rbx   ; save rbx
    mov QWORD [rax+16], rcx  ; save rcx
    mov QWORD [rax+24], rdx  ; save rdx
    mov QWORD [rax+32], rsi  ; save rsi
    mov QWORD [rax+40], rdi  ; save rdi
    mov QWORD [rax+56], rbp  ; save rbp
    mov QWORD [rax+64],  r8  ; save r8
    mov QWORD [rax+72],  r9  ; save r9
    mov QWORD [rax+80], r10  ; save r10
    mov QWORD [rax+88], r11  ; save r11
    mov QWORD [rax+96], r12  ; save r12
    mov QWORD [rax+104], r13 ; save r13
    mov QWORD [rax+112], r14 ; save r14
    mov QWORD [rax+120], r15 ; save r15

    pop rbx
    mov QWORD [rax], rbx ; save rax

    pop rbx
    mov QWORD [rax+128], rbx ; save RIP

    pop rbx ; get rid of saving address

    mov rbx, QWORD [rax+128]
    push rbx ; place return address back on stack

    mov QWORD [rax+48], rsp ; finally can save rsp properly

    xor rbx, rbx
    mov bx, cs
    mov WORD [rax+136], bx ; save cs

    xor rbx, rbx
    mov bx, ds
    mov WORD [rax+138], bx ; save ds

    pushf
    pop rbx
    mov QWORD [rax+140], rbx ; save rflags

    mov rbx, cr3
    mov QWORD [rax+148], rbx ; save cr3

    mov rbx, QWORD [rax+8] ; restore rbx
    mov rax, QWORD [rax]   ; restore rax

    sti ; enable interrupts
    ret

global x86_64_get_stack_ptr
x86_64_get_stack_ptr:
    mov rax, rsp
    ret

extern kernel_stack
extern kernel_stack_size

global x86_64_kernel_thread_end
x86_64_kernel_thread_end:
    cli
    pop rax
    mov rsp, kernel_stack
    add rsp, [kernel_stack_size]
    xor rbp, rbp
    push rax
    ret
