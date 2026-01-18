load_kernel:
    mov ah, 0x02        ; BIOS Read Sector
    mov al, 15          ; Read 15 sectors
    mov ch, 0x00        ; Cylinder 0
    mov dh, 0x00        ; Head 0
    mov cl, 0x02        ; Start at Sector 2
    
    xor bx, bx          ; Clear BX
    mov es, bx          ; ES = 0
    mov bx, KERNEL_OFFSET ; BX = 0x1000. Buffer = 0000:1000
    
    mov dl, [BOOT_DRIVE]; Ensure we read from the boot disk
    int 0x13            ; BIOs Interrupt
    
    jc disk_error       ; Check Carry Flag (BIOS Error)
    
    cmp al, 15          ; Check Sector Count (Incomplete Read)
    jne disk_error
    
    ret
