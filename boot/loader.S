%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR

;build GDT
GDT_BASE:   dd  0x00000000
            dd  0x00000000 ; 第0个描述符无用

CODE_DESC:  dd  0x0000FFFF
            dd  DESC_CODE_HIGH4

DATA_STACK_DESC:    dd  0x0000FFFF
                    dd  DESC_DATA_HIGH4

VIDEO_DESC: dd  0x80000007
            dd  DESC_VIDEO_HIGH4 

GDT_SIZE    equ $ - GDT_BASE
GDT_LIMIT   equ GDT_SIZE - 1
times   60  dq  0
total_mem_bytes dd 0

SELECTOR_CODE   equ (0x0001<<3) + TI_GDT + RPL0
SELECTOR_DATA   equ (0x0002<<3) + TI_GDT + RPL0
SELECTOR_VIDEO   equ (0x0003<<3) + TI_GDT + RPL0

gdt_ptr dw  GDT_LIMIT
        dd  GDT_BASE

ards_buf times 244 db 0
ards_nr dw 0

loadermsg   db  'AOS LOADER in real.'

loader_start:
    
    xor ebx, ebx
    mov edx, 0x534d4150
    mov di, ards_buf
.e820_mem_get_loop:
    mov eax, 0x000e820
    mov ecx, 20
    int 0x15
    add di, cx
    inc word [ards_nr]
    cmp ebx, 0
    jnz .e820_mem_get_loop

    mov cx, [ards_nr]
    mov ebx, ards_buf
    xor edx, edx
.find_max_mem_area:
    mov eax, [ebx]
    add eax, [ebx+8]
    ;mov eax, [ebx+8]
    add ebx, 20
    cmp edx, eax
    jge .next_ards
    mov edx, eax
.next_ards:
    loop .find_max_mem_area
    jmp .mem_get_ok


.mem_get_ok:
    mov [total_mem_bytes], edx


    mov sp, LOADER_BASE_ADDR
    mov bp, loadermsg
    mov cx, 19
    mov ax, 0x1301
    mov bx, 0x001f
    mov dx, 0x1800
    int 0x10

    ; open A20
    in al, 0x92
    or al, 0000_0010B
    out 0x92, al

    ;load GDT
    lgdt [gdt_ptr]

    ;set cr0 0th bit to 1, go into protecting mode
    mov eax, cr0
    or  eax, 0x00000001
    mov cr0, eax

    jmp dword SELECTOR_CODE:p_mode_start

[bits 32]
p_mode_start:
    mov ax, SELECTOR_DATA
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, LOADER_STACK_TOP
    mov ax, SELECTOR_VIDEO
    mov gs, ax

    mov byte [gs:160], 'p'

    jmp $
