[bits 64]

extern x86_64_Panic

global x86_64_PrePanic
x86_64_PrePanic:
    push rbp
    mov rbp, rsp
    pushf
    cli ; must be done after flags are saved
    push rax
    mov rax, cr3
    sub rsp, 4 ; used to fix alignment later
    push rax
    push QWORD [rbp-8] ; RFLAGS
    sub rsp, 2
    mov WORD [rsp], ds
    sub rsp, 2
    mov WORD [rsp], cs
    push QWORD [rbp+8] ; RIP is placed on stack at call
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push QWORD [rbp] ; get rbp
    mov rax, rbp ; get rsp
    sub rax, 24
    push rax
    push rdi
    push rsi
    push rdx
    push rcx
    push rbx
    push QWORD [rbp-16]
    ;lea rdi, QWORD [rbp+16]
    xor rdi, rdi
    mov rsi, rsp
    xor rdx, rdx
    call x86_64_Panic
.l:
    hlt
    jmp .l
