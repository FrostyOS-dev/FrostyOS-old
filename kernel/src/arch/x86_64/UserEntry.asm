[bits 64]

extern main
extern __user_exit

global _start
_start:
    xor rbp, rbp
    pop rdi ; get argc off stack
    mov rsi, rsp ; argv is at top of stack now
    and rsp, ~15
    call main
    mov rdi, rax
    call __user_exit
    hlt
