[bits 64]

global x86_64_TSS_Load
x86_64_TSS_Load:
    ltr di
    ret
