[BITS 16]
[ORG 0x7C00]

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x8000

    mov [BOOT_DRIVE], dl

    ; Read kernel
    mov ah, 0x02
    mov al, 45
    mov ch, 0
    mov dh, 0
    mov cl, 2
    mov bx, 0x1000
    int 0x13
    jc disk_error

    ; Enable A20
    in al, 0x92
    or al, 2
    out 0x92, al

    cli
    ; Load GDT and switch to PM
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    jmp CODE_SEG:init_pm

disk_error:
    mov ah, 0x0E
    mov al, 'E'
    int 0x10
    jmp $

[BITS 32]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov ebp, 0x90000
    mov esp, ebp

    call 0x1000
    jmp $

; GDT configuration
gdt_start:

gdt_null:
    dd 0x0
    dd 0x0

gdt_code:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10011010b
    db 11001111b
    db 0x0

gdt_data:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start
BOOT_DRIVE db 0

times 446 - ($ - $$) db 0

; add a partition table
db 0x80             ; bootable
db 0x00, 0x01, 0x00 ; CHS start
db 0x0C             ; part type
db 0x00, 0x01, 0x00 ; CHS end
dd 2048             ; LBA
dd 131072           ; sectors

times 510 - ($ - $$) db 0

dw 0xAA55
