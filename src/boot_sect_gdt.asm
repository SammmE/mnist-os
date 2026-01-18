gdt_start:

gdt_null:               ; null descriptor
    dd 0x0
    dd 0x0

gdt_code:               ; The Code Segment Descriptor
    ; base=0x0, limit=0xfffff, 1st flags: (present)1 (privilege)00 (descriptor type)1 -> 1001b
    ; type flags: (code)1 (conforming)0 (readable)1 (accessed)0 -> 1010b
    ; 2nd flags: (granularity)1 (32-bit default)1 (64-bit seg)0 (AVL)0 -> 1100b
    dw 0xffff           ; Limit (bits 0-15)
    dw 0x0              ; Base (bits 0-15)
    db 0x0              ; Base (bits 16-23)
    db 10011010b        ; 1st flags, type flags
    db 11001111b        ; 2nd flags, Limit (bits 16-19)
    db 0x0              ; Base (bits 24-31)

gdt_data:               ; The Data Segment Descriptor
    ; Same as code segment except for the type flags:
    ; type flags: (code)0 (expand down)0 (writable)1 (accessed)0 -> 0010b
    dw 0xffff
    dw 0x0
    db 0x0
    db 10010010b        ; 1st flags, type flags
    db 11001111b        ; 2nd flags, Limit (bits 16-19)
    db 0x0

gdt_end:                ; Used to calculate the size of the GDT

gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; Size (16 bit)
    dd gdt_start               ; Start Address (32 bit)

; Define constants for the GDT segment offsets
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start
