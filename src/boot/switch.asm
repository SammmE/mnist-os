[bits 16]
switch_to_pm:
    cli                     ; Disable interrupts
    lgdt [gdt_descriptor]   ; Load the GDT desc

    mov eax, cr0
    or eax, 0x1             ; Set 32-bit mode
    mov cr0, eax

    jmp CODE_SEG:init_pm    ; Far jump to get rid of cache

[bits 32]
init_pm:
    mov ax, DATA_SEG        ; Update registers
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000        ; Update stack position to top of free space
    mov esp, ebp

    call BEGIN_PM
