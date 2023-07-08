[bits 64]

global x86_64_LoadGDT
x86_64_LoadGDT:
    lgdt [rdi]

    mov ax, 0x10 ; set the new data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    pop rcx ; save return address

    mov rax, 0x08 ; prepare new code segment
    push rax

    push rcx ; restore return address

    retfq ; far return into new code segment
