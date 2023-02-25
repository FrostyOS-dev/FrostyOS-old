[bits 64]

global InitKernelStack
InitKernelStack:
    cli
    pop rax
    add rdi, rsi
    mov rsp, rdi
    mov rbp, rsp
    push rax
    sti
    ret