[bits 64]

global CombineDWords
CombineDWords:
    xor rax, rax
    mov eax, edi
    shl rax, 32
    mov eax, esi
    ret

global CombineWords
CombineWords:
    xor rax, rax
    mov ax, di
    shl eax, 16
    mov ax, si
    ret

global CombineBytes
CombineBytes:
    xor rax, rax
    mov al, dil
    shl ax, 8
    mov al, sil
    ret
