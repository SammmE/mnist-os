[bits 32]
[extern kmain]

global _start
_start:
    call kmain          ; Call the C function
    jmp $               ; Infinite loop if C returns
