; Copyright (©) 2022-2023  Frosty515
; 
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
; 
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
; 
; You should have received a copy of the GNU General Public License
; along with this program.  If not, see <https://www.gnu.org/licenses/>.

[bits 64]

global x86_64_FlushTLB
x86_64_FlushTLB:
    cli
    mov rax, cr3 ; save cr3
    ;mov rcx, 0   ; use rcx to place a temperary value into cr3
    ;mov cr3, rcx
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

global x86_64_SwapCR3
x86_64_SwapCR3:
    mov rax, cr3
    mov cr3, rdi
    ret

global x86_64_GetCR2
x86_64_GetCR2:
    mov rax, cr2
    ret

global x86_64_EnsureNX
x86_64_EnsureNX:
    mov rcx, 0xC0000080 ; EFER MSR
    rdmsr ; we can assume that the register exists or we would not be in long mode
    and rax, 2048 ; get bit 11
    shr rax, 11
    cmp al, 1
    jz .success
    mov rax, 0x80000001 ; put value in rax for NX check
    cpuid ; we can assume cpuid is supported or we would not be in long mode
    and edx, 1048576 ; select NX check bit
    shr edx, 20
    cmp edx, 1
    jnz .fail
    mov rcx, 0xC0000080 ; EFER MSR
    rdmsr
    or eax, 2048 ; bit 11
    wrmsr
.success:
    mov rax, 1
    ret
.fail:
    xor rax, rax ; clear rax
    ret

global x86_64_EnsureLargePages
x86_64_EnsureLargePages:
    push rbp
    mov rbp, rsp

    mov rax, cr4
    and rax, 16 ; get bit 4
    shr rax, 4
    cmp al, 1
    jz .success

    mov rax, 1
    xor rcx, rcx
    push rbx
    xor rbx, rbx
    xor rdx, rdx
    cpuid
    pop rbx
    and rdx, 8
    shr rdx, 3
    cmp dl, 1
    jnz .fail

    mov rax, cr4
    or rax, 16
    mov cr4, rax

.success:
    mov rax, 1
    jmp .end

.fail:
    mov rax, 0

.end:
    mov rsp, rbp
    pop rbp
    ret
