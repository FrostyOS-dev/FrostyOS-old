[bits 64]

segment .data

; We should be loading at 0x0000

global ap_trampoline
ap_trampoline:
[bits 16]
    cli
    cld
    jmp near 0x0060
align 16
.pmode_gdt: ; 0x0010
    dq 0 ; null selector
    dq 0x00CF9A000000FFFF ; 32-bit code
    dq 0x00CF92000000FFFF ; 32-bit data
    dq 0x00AF9A000000FFFF ; 64-bit code
    dq 0x00AF92000000FFFF ; 64-bit data
.pmode_gdtr: ; 0x0038
    dw 39
    dq 0x00000010
align 32
.start: ; 0x0060
    xor eax, eax
    mov ds, ax
    mov ax, 0x0038
    lgdt [eax]
    mov eax, cr0
    or eax, 1 ; PE
    mov cr0, eax
    mov esp, 0xFD0 ; starts 48 bytes from end of page, grows down
    jmp 0x08:dword 0x0080
align 64
[bits 32]
.pmode: ; 0x0080, we are in protected mode now. time to get ready for long mode
    mov eax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov eax, cr4
    or eax, 1<<5 ; PAE
    mov cr4, eax
    mov eax, [0xFFC] ; last 4 bytes of the page is the physical address of the kernel page table
    mov cr3, eax
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1<<8 ; LME
    wrmsr
    mov eax, cr0
    and eax, ~(1 << 30 | 1 << 29) ; clear CD and NW
    or eax, 1<<31 | 1<<16 ; PG, WP
    mov cr0, eax
    jmp 0x18:dword 0x120
align 32
[bits 64]
.lmode: ; 0x0120, we are now in long mode. Now we just setup what the kernel needs
    mov rax, 0x20
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov rax, x86_64_EnsureNX
    push rax
    call near QWORD [rsp]
    test rax, rax
    jz 0x0200
    mov QWORD [rsp], x86_64_EnsureLargePages
    call near QWORD [rsp]
    pop rcx
    test rax, rax
    jz 0x0200
    lock inc QWORD [aps_running]
.spin:
    mov eax, dword [0xFF8] ; start lock
    test eax, eax
    jnz .spin ; should be a relative jump
    mov rax, QWORD [0xFF0]
    mov rdi, QWORD [0xFE8]
    mov rsp, QWORD [0xFE0]
    mov rcx, QWORD [0xFD8]
    mov QWORD [rdi+rcx], rsp
    add rsp, 16384
    xor rsi, rsi
    xor rdx, rdx
    xor rcx, rcx
    call rax ; shouldn't return, but if we do, we just fall through to .lmode_fail

align 256
.lmode_fail: ; 0x0200
    mov al, 'F'
    out 0xE9, al
    mov al, 'A'
    out 0xE9, al
    mov al, 'I'
    out 0xE9, al
    mov al, 'L'
    out 0xE9, al
    jmp $

align 4096
.end:

extern x86_64_EnsureNX
extern x86_64_EnsureLargePages

extern aps_running
