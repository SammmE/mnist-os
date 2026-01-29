; boot_sect_mem.asm - BIOS E820 Memory Detection
; Detects available memory using BIOS INT 15h, EAX=E820h
; Stores memory map entries at MEMORY_MAP_ADDR (0x0500)

MEMORY_MAP_ADDR equ 0x0500
MEMORY_MAP_COUNT_ADDR equ 0x04FC  ; Store count of entries here (4 bytes before map)

detect_memory:
    pusha
    
    ; Initialize
    xor ebx, ebx                    ; EBX = 0 (first call)
    xor bp, bp                      ; BP = entry count
    mov di, MEMORY_MAP_ADDR         ; DI = destination buffer
    mov edx, 0x534D4150             ; EDX = 'SMAP' signature
    
.loop:
    mov eax, 0xE820                 ; Function E820
    mov ecx, 24                     ; Buffer size (24 bytes)
    int 0x15                        ; Call BIOS
    
    jc .done                        ; CF set = error or done
    
    cmp eax, 0x534D4150             ; Verify 'SMAP' signature
    jne .failed
    
    ; Entry retrieved successfully
    add di, 24                      ; Move to next entry slot
    inc bp                          ; Increment entry count
    
    test ebx, ebx                   ; If EBX = 0, we're done
    jz .done
    
    cmp bp, 32                      ; Limit to 32 entries (safety)
    jge .done
    
    jmp .loop
    
.done:
    ; Store entry count
    mov dword [MEMORY_MAP_COUNT_ADDR], ebp
    
    ; Print success message
    mov bx, MSG_MEM_DETECT_OK
    call print_string
    
    popa
    ret
    
.failed:
    ; Print error message
    mov bx, MSG_MEM_DETECT_FAIL
    call print_string
    
    ; Store 0 entries
    xor eax, eax
    mov dword [MEMORY_MAP_COUNT_ADDR], eax
    
    popa
    ret

MSG_MEM_DETECT_OK   db "Memory detected. ", 0
MSG_MEM_DETECT_FAIL db "Memory detection failed! ", 0
