[bits 16]
[org 0x7c00]

KERNEL_OFFSET equ 0x1000

; save boot drive
mov [BOOT_DRIVE], dl

; stack setup
mov bp, 0x9000
mov sp, bp

; load kernel
mov bx, MSG_LOAD_KERNEL
call print_string

call switch_to_protected_mode

jmp $

%include "src/boot_sect_disk.asm"
%include "src/boot_sect_print.asm"
%include "src/boot_sect_gdt.asm"
%include "src/boot_sect_switch.asm"

[bits 32]
BEGIN_PROTECTED_MODE:
    ; jump to kernel entry point
    call KERNEL_OFFSET:0x0000
    jmp $

BOOT_DRIVE      db 0
MSG_LOAD_KERNEL db "Loading Kernel...", 0
MSG_SWITCH_PM   db "Switching to 32-bit...", 0

times 510-($-$$) db 0
dw 0xaa55
