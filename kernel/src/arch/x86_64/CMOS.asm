[bits 64]

global CMOS_Read
CMOS_Read:
    mov al, dil
    out 0x70, al
    in al, 0x71
    ret

global CMOS_Write
CMOS_Write:
    mov al, dil
    out 0x70, al
    mov al, sil
    out 0x71, al
    ret