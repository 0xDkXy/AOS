%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR

    mov ax, message
    mov bp, ax

    mov cx, 10
    mov ax, 0x1301
    mov bx, 0x2
    int 0x10

    jmp $

    message db "AOS LOADER"
