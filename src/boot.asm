[org 0x7C00]
KERNEL_OFFSET equ 0x1000

    mov [BOOT_DRIVE], dl 

    mov ax, 0x9000
    mov ss, ax
    mov sp, 0xFFFF
    mov bp, sp
    xor ax, ax
    mov ds, ax
    mov es, ax

    mov bx, MSG_REAL_MODE
    call print_string

    call load_kernel
    call enable_a20

    mov ah, 0x00
    mov al, 0x13
    int 0x10

    call switch_to_pm
    jmp $

%include "src/print_string.asm"
%include "src/disk_load.asm"
%include "src/gdt.asm"
%include "src/print_string_pm.asm"
%include "src/switch_to_pm.asm"

[bits 16]
enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

load_kernel:
    mov bx, MSG_LOAD_KERNEL
    call print_string

    mov bx, KERNEL_OFFSET
    mov dh, 0
    mov dl, [BOOT_DRIVE]
    mov ch, 0
    mov cl, 0x02
    
    mov ah, 0x02
    mov al, 50      ; Read 50 sectors
    int 0x13
    
    jc disk_error
    ret

load_next_chunk:
    cmp si, 0
    jle finish_load         ; If SI <= 0, we are done

    mov al, 50              ; Read 50 sectors at a time (safe size)
    cmp si, 50
    jge .do_read
    mov ax, si              ; If < 50 left, just read remainder (move to AL)

.do_read:
    mov ah, 0x02            ; BIOS Read Sector
    int 0x13
    jc disk_error           ; Panic if error

    sub si, 50              ; Decrease total remaining
    add bx, 0x6400          
    add cl, 50              
    jmp load_next_chunk

finish_load:
    ret

[bits 32]
BEGIN_PM:
    mov ebx, MSG_PROT_MODE
    call print_string_pm

    call KERNEL_OFFSET

    jmp $

; Global variables
BOOT_DRIVE db 0
MSG_REAL_MODE db "Started in 16-bit Real Mode", 0
MSG_PROT_MODE db "Successfully landed in 32-bit Protected Mode", 0
MSG_LOAD_KERNEL db "Loading kernel into memory.", 0

; Bootsector padding
times 510-($-$$) db 0
dw 0xaa55
