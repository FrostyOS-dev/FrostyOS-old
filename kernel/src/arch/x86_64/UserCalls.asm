[bits 64]

%macro USERCALL 2
global __user_%1
__user_%1:
    mov rax, %2
    syscall
    ret

%endmacro

%macro USERCALL_MOREARGS 2
global __user_%1
__user_%1:
    mov rax, %2
    mov r10, rcx
    syscall
    ret

%endmacro

USERCALL read,0
USERCALL write,1
USERCALL open,2
USERCALL close,3
USERCALL stat,4
USERCALL fstat,5
USERCALL seek,8
USERCALL_MOREARGS mmap,9
USERCALL mprotect,10
USERCALL munmap,11
USERCALL nanosleep,35
USERCALL exit,60
USERCALL time,201
