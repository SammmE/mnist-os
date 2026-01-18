print_string:
    pusha               ; Save all registers
    
    mov ah, 0x0E        ; INT 10h / AH=0Eh: Teletype Output
    xor bh, bh          ; Page Number = 0 (Ensure we draw to visible screen)
    mov bl, 0x07        ; Color = Light Grey (Used only in Graphics Mode)

.loop:
    mov al, [bx]        ; Get char at DS:BX
    or al, al           ; Check if AL == 0, pretty sure this will work better
    jz .done            ; If zero, we are done
    
    int 0x10            ; Call BIOS to print AL
    
    inc bx              ; Point to next char
    jmp .loop           ; Repeat

.done:
    popa                ; Restore registers
    ret
