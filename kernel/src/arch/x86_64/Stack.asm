[bits 64]

global InitKernelStack
InitKernelStack:
    pop rax
    add rdi, rsi
    mov rsp, rdi
    mov rbp, rsp
    push rax
    ret