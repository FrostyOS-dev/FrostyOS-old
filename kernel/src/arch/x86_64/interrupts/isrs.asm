; Copyright (©) 2022-2024  Frosty515
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

extern x86_64_ISR_Handler

; cpu pushes to the stack: rflags, cs, rip

%macro ISR_NOERRORCODE 1

global x86_64_ISR%1:
x86_64_ISR%1:
    push 0              ; push dummy error code
    push %1             ; push interrupt number
    jmp isr_common

%endmacro

%macro ISR_ERRORCODE 1
global x86_64_ISR%1:
x86_64_ISR%1:
                        ; cpu pushes an error code to the stack
    push %1             ; push interrupt number
    jmp isr_common

%endmacro

%macro pushaq 0
    push rax
    push rcx
    push rbx
    push rdx
    push rsp
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro popaq 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rsp
    pop rdx
    pop rbx
    pop rcx
    pop rax
%endmacro

ISR_NOERRORCODE 0
ISR_NOERRORCODE 1
ISR_NOERRORCODE 2
ISR_NOERRORCODE 3
ISR_NOERRORCODE 4
ISR_NOERRORCODE 5
ISR_NOERRORCODE 6
ISR_NOERRORCODE 7
ISR_ERRORCODE 8
ISR_NOERRORCODE 9
ISR_ERRORCODE 10
ISR_ERRORCODE 11
ISR_ERRORCODE 12
ISR_ERRORCODE 13
ISR_ERRORCODE 14
ISR_NOERRORCODE 15
ISR_NOERRORCODE 16
ISR_ERRORCODE 17
ISR_NOERRORCODE 18
ISR_NOERRORCODE 19
ISR_NOERRORCODE 20
ISR_ERRORCODE 21
ISR_NOERRORCODE 22
ISR_NOERRORCODE 23
ISR_NOERRORCODE 24
ISR_NOERRORCODE 25
ISR_NOERRORCODE 26
ISR_NOERRORCODE 27
ISR_NOERRORCODE 28
ISR_ERRORCODE 29
ISR_ERRORCODE 30
ISR_NOERRORCODE 31
ISR_NOERRORCODE 32
ISR_NOERRORCODE 33
ISR_NOERRORCODE 34
ISR_NOERRORCODE 35
ISR_NOERRORCODE 36
ISR_NOERRORCODE 37
ISR_NOERRORCODE 38
ISR_NOERRORCODE 39
ISR_NOERRORCODE 40
ISR_NOERRORCODE 41
ISR_NOERRORCODE 42
ISR_NOERRORCODE 43
ISR_NOERRORCODE 44
ISR_NOERRORCODE 45
ISR_NOERRORCODE 46
ISR_NOERRORCODE 47
ISR_NOERRORCODE 48
ISR_NOERRORCODE 49
ISR_NOERRORCODE 50
ISR_NOERRORCODE 51
ISR_NOERRORCODE 52
ISR_NOERRORCODE 53
ISR_NOERRORCODE 54
ISR_NOERRORCODE 55
ISR_NOERRORCODE 56
ISR_NOERRORCODE 57
ISR_NOERRORCODE 58
ISR_NOERRORCODE 59
ISR_NOERRORCODE 60
ISR_NOERRORCODE 61
ISR_NOERRORCODE 62
ISR_NOERRORCODE 63
ISR_NOERRORCODE 64
ISR_NOERRORCODE 65
ISR_NOERRORCODE 66
ISR_NOERRORCODE 67
ISR_NOERRORCODE 68
ISR_NOERRORCODE 69
ISR_NOERRORCODE 70
ISR_NOERRORCODE 71
ISR_NOERRORCODE 72
ISR_NOERRORCODE 73
ISR_NOERRORCODE 74
ISR_NOERRORCODE 75
ISR_NOERRORCODE 76
ISR_NOERRORCODE 77
ISR_NOERRORCODE 78
ISR_NOERRORCODE 79
ISR_NOERRORCODE 80
ISR_NOERRORCODE 81
ISR_NOERRORCODE 82
ISR_NOERRORCODE 83
ISR_NOERRORCODE 84
ISR_NOERRORCODE 85
ISR_NOERRORCODE 86
ISR_NOERRORCODE 87
ISR_NOERRORCODE 88
ISR_NOERRORCODE 89
ISR_NOERRORCODE 90
ISR_NOERRORCODE 91
ISR_NOERRORCODE 92
ISR_NOERRORCODE 93
ISR_NOERRORCODE 94
ISR_NOERRORCODE 95
ISR_NOERRORCODE 96
ISR_NOERRORCODE 97
ISR_NOERRORCODE 98
ISR_NOERRORCODE 99
ISR_NOERRORCODE 100
ISR_NOERRORCODE 101
ISR_NOERRORCODE 102
ISR_NOERRORCODE 103
ISR_NOERRORCODE 104
ISR_NOERRORCODE 105
ISR_NOERRORCODE 106
ISR_NOERRORCODE 107
ISR_NOERRORCODE 108
ISR_NOERRORCODE 109
ISR_NOERRORCODE 110
ISR_NOERRORCODE 111
ISR_NOERRORCODE 112
ISR_NOERRORCODE 113
ISR_NOERRORCODE 114
ISR_NOERRORCODE 115
ISR_NOERRORCODE 116
ISR_NOERRORCODE 117
ISR_NOERRORCODE 118
ISR_NOERRORCODE 119
ISR_NOERRORCODE 120
ISR_NOERRORCODE 121
ISR_NOERRORCODE 122
ISR_NOERRORCODE 123
ISR_NOERRORCODE 124
ISR_NOERRORCODE 125
ISR_NOERRORCODE 126
ISR_NOERRORCODE 127
ISR_NOERRORCODE 128
ISR_NOERRORCODE 129
ISR_NOERRORCODE 130
ISR_NOERRORCODE 131
ISR_NOERRORCODE 132
ISR_NOERRORCODE 133
ISR_NOERRORCODE 134
ISR_NOERRORCODE 135
ISR_NOERRORCODE 136
ISR_NOERRORCODE 137
ISR_NOERRORCODE 138
ISR_NOERRORCODE 139
ISR_NOERRORCODE 140
ISR_NOERRORCODE 141
ISR_NOERRORCODE 142
ISR_NOERRORCODE 143
ISR_NOERRORCODE 144
ISR_NOERRORCODE 145
ISR_NOERRORCODE 146
ISR_NOERRORCODE 147
ISR_NOERRORCODE 148
ISR_NOERRORCODE 149
ISR_NOERRORCODE 150
ISR_NOERRORCODE 151
ISR_NOERRORCODE 152
ISR_NOERRORCODE 153
ISR_NOERRORCODE 154
ISR_NOERRORCODE 155
ISR_NOERRORCODE 156
ISR_NOERRORCODE 157
ISR_NOERRORCODE 158
ISR_NOERRORCODE 159
ISR_NOERRORCODE 160
ISR_NOERRORCODE 161
ISR_NOERRORCODE 162
ISR_NOERRORCODE 163
ISR_NOERRORCODE 164
ISR_NOERRORCODE 165
ISR_NOERRORCODE 166
ISR_NOERRORCODE 167
ISR_NOERRORCODE 168
ISR_NOERRORCODE 169
ISR_NOERRORCODE 170
ISR_NOERRORCODE 171
ISR_NOERRORCODE 172
ISR_NOERRORCODE 173
ISR_NOERRORCODE 174
ISR_NOERRORCODE 175
ISR_NOERRORCODE 176
ISR_NOERRORCODE 177
ISR_NOERRORCODE 178
ISR_NOERRORCODE 179
ISR_NOERRORCODE 180
ISR_NOERRORCODE 181
ISR_NOERRORCODE 182
ISR_NOERRORCODE 183
ISR_NOERRORCODE 184
ISR_NOERRORCODE 185
ISR_NOERRORCODE 186
ISR_NOERRORCODE 187
ISR_NOERRORCODE 188
ISR_NOERRORCODE 189
ISR_NOERRORCODE 190
ISR_NOERRORCODE 191
ISR_NOERRORCODE 192
ISR_NOERRORCODE 193
ISR_NOERRORCODE 194
ISR_NOERRORCODE 195
ISR_NOERRORCODE 196
ISR_NOERRORCODE 197
ISR_NOERRORCODE 198
ISR_NOERRORCODE 199
ISR_NOERRORCODE 200
ISR_NOERRORCODE 201
ISR_NOERRORCODE 202
ISR_NOERRORCODE 203
ISR_NOERRORCODE 204
ISR_NOERRORCODE 205
ISR_NOERRORCODE 206
ISR_NOERRORCODE 207
ISR_NOERRORCODE 208
ISR_NOERRORCODE 209
ISR_NOERRORCODE 210
ISR_NOERRORCODE 211
ISR_NOERRORCODE 212
ISR_NOERRORCODE 213
ISR_NOERRORCODE 214
ISR_NOERRORCODE 215
ISR_NOERRORCODE 216
ISR_NOERRORCODE 217
ISR_NOERRORCODE 218
ISR_NOERRORCODE 219
ISR_NOERRORCODE 220
ISR_NOERRORCODE 221
ISR_NOERRORCODE 222
ISR_NOERRORCODE 223
ISR_NOERRORCODE 224
ISR_NOERRORCODE 225
ISR_NOERRORCODE 226
ISR_NOERRORCODE 227
ISR_NOERRORCODE 228
ISR_NOERRORCODE 229
ISR_NOERRORCODE 230
ISR_NOERRORCODE 231
ISR_NOERRORCODE 232
ISR_NOERRORCODE 233
ISR_NOERRORCODE 234
ISR_NOERRORCODE 235
ISR_NOERRORCODE 236
ISR_NOERRORCODE 237
ISR_NOERRORCODE 238
ISR_NOERRORCODE 239
ISR_NOERRORCODE 240
ISR_NOERRORCODE 241
ISR_NOERRORCODE 242
ISR_NOERRORCODE 243
ISR_NOERRORCODE 244
ISR_NOERRORCODE 245
ISR_NOERRORCODE 246
ISR_NOERRORCODE 247
ISR_NOERRORCODE 248
ISR_NOERRORCODE 249
ISR_NOERRORCODE 250
ISR_NOERRORCODE 251
ISR_NOERRORCODE 252
ISR_NOERRORCODE 253
ISR_NOERRORCODE 254
ISR_NOERRORCODE 255

isr_common:
    pushaq

    mov rax, cr2        ; push CR2
    push rax

    mov rax, cr3        ; push CR3
    push rax

    xor rax, rax        ; push ds
    mov ax, ds
    push rax

    mov ax, 0x10        ; use kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov r15, rsp

    push QWORD [r15+168] ; rip
    push rbp
    mov rbp, rsp

    lea rdi, QWORD [rsp+16]
    
    call x86_64_ISR_Handler

    add rsp, 16 ; remove rip, rbp

    pop rax             ; restore old segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    pop rax             ; remove cr3
    mov cr3, rax
    pop rax             ; remove cr2
    mov cr2, rax

    popaq
    add rsp, 16         ; remove error code and interrupt number
    iretq               ; will pop: cs, rip, rflags, ss, rsp
    