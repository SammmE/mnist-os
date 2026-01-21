[bits 32]
[extern kmain]      ; Define that kmain exists somewhere else (in C)

global _start
_start:
    call kmain          ; Call the C function
    jmp $               ; Infinite loop if C returns
