[bits 64]

global spinlock_acquire
spinlock_acquire:
    lock bts QWORD [rdi], 0
    jc .spin_with_pause
    ret

.spin_with_pause:
    pause
    test QWORD [rdi], 1
    jnz .spin_with_pause
    jmp spinlock_acquire

global spinlock_release
spinlock_release:
    mov QWORD [rdi], 0
    ret