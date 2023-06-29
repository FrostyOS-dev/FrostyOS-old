[bits 64]

global x86_64_outb
x86_64_outb:
    mov al, sil ; value must be in al or the instruction won't work
    mov dx, di ; port must be in dx or instruction won't work
    out dx, al
    ret

global x86_64_inb
x86_64_inb:
    mov dx, di ; port must be in dx
    xor rax, rax ; clear rax
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

global x86_64_iowait
x86_64_iowait:
    xor rax, rax ; clear rax
    out 0x80, al ; 0x80 is an unused port
    ret
