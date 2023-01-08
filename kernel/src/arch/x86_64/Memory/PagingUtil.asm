[bits 64]

global x86_64_FlushTLB
x86_64_FlushTLB:
    cli
    mov rax, cr3 ; save cr3
    mov rcx, 0   ; use rcx to place a temperary value into cr3
    mov cr3, rcx
    mov cr3, rax ; restore cr3
    sti
    ret

global x86_64_LoadCR3
x86_64_LoadCR3:
    mov cr3, rdi
    ret

global x86_64_GetCR3
x86_64_GetCR3:
    mov rax, cr3
    ret
