[bits 64]

extern kernel_stack
extern kernel_stack_size
extern _start

global __kernel_start
__kernel_start:
    cli
    mov rsp, kernel_stack
    add rsp, [kernel_stack_size]
    xor rbp, rbp
    call _start
.end:
    hlt
    jmp .end
