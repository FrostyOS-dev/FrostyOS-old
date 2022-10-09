[bits 64]

global x86_64_outb
x86_64_outb:
    mov dx, [esp + 4]
    mov al, [esp + 8]
    out dx, al
    ret

global x86_64_inb
x86_64_inb:
    mov dx, [esp + 4]
    xor rax, rax
    in al, dx
    ret

global x86_64_EnableInterrupts
x86_64_EnableInterrupts:
    sti
    ret

global x86_64_DisableInterrupts
x86_64_DisableInterrupts:
    cli
    ret
