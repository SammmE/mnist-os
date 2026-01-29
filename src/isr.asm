; Interrupt Service Routines (ISRs) for x86 protected mode
; This file contains all ISR and IRQ stubs that save state and call C handlers

[bits 32]

; External C functions
extern isr_handler
extern irq_handler

; Common ISR stub - saves processor state, calls C handler, restores state
; This is called by all ISR stubs
isr_common_stub:
    ; Save all registers
    pusha                   ; Pushes edi, esi, ebp, esp, ebx, edx, ecx, eax
    
    mov ax, ds              ; Lower 16-bits of eax = ds
    push eax                ; Save the data segment descriptor
    
    mov ax, 0x10            ; Load the kernel data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    call isr_handler        ; Call C handler
    
    pop eax                 ; Restore the original data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    popa                    ; Restore registers
    add esp, 8              ; Clean up pushed error code and ISR number
    sti                     ; Re-enable interrupts
    iret                    ; Return from interrupt (pops CS, EIP, EFLAGS, SS, ESP)

; Common IRQ stub - similar to ISR but calls irq_handler
irq_common_stub:
    pusha
    
    mov ax, ds
    push eax
    
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    call irq_handler        ; Call C IRQ handler
    
    pop ebx
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx
    
    popa
    add esp, 8
    sti
    iret

; Macro to create ISR stubs without error codes
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    cli                     ; Disable interrupts
    push 0                  ; Push dummy error code
    push %1                 ; Push interrupt number
    jmp isr_common_stub
%endmacro

; Macro to create ISR stubs with error codes (CPU pushes them automatically)
%macro ISR_ERRCODE 1
global isr%1
isr%1:
    cli
    push %1                 ; Push interrupt number (error code already pushed by CPU)
    jmp isr_common_stub
%endmacro

; Macro to create IRQ stubs
%macro IRQ 2
global irq%1
irq%1:
    cli
    push 0                  ; Push dummy error code
    push %2                 ; Push interrupt number (32 + IRQ number)
    jmp irq_common_stub
%endmacro

; CPU Exception ISRs (0-31)
ISR_NOERRCODE 0     ; Division By Zero
ISR_NOERRCODE 1     ; Debug
ISR_NOERRCODE 2     ; Non Maskable Interrupt
ISR_NOERRCODE 3     ; Breakpoint
ISR_NOERRCODE 4     ; Overflow
ISR_NOERRCODE 5     ; Bound Range Exceeded
ISR_NOERRCODE 6     ; Invalid Opcode
ISR_NOERRCODE 7     ; Device Not Available
ISR_ERRCODE   8     ; Double Fault (with error code)
ISR_NOERRCODE 9     ; Coprocessor Segment Overrun
ISR_ERRCODE   10    ; Invalid TSS (with error code)
ISR_ERRCODE   11    ; Segment Not Present (with error code)
ISR_ERRCODE   12    ; Stack-Segment Fault (with error code)
ISR_ERRCODE   13    ; General Protection Fault (with error code)
ISR_ERRCODE   14    ; Page Fault (with error code)
ISR_NOERRCODE 15    ; Reserved
ISR_NOERRCODE 16    ; x87 Floating-Point Exception
ISR_ERRCODE   17    ; Alignment Check (with error code)
ISR_NOERRCODE 18    ; Machine Check
ISR_NOERRCODE 19    ; SIMD Floating-Point Exception
ISR_NOERRCODE 20    ; Virtualization Exception
ISR_NOERRCODE 21    ; Reserved
ISR_NOERRCODE 22    ; Reserved
ISR_NOERRCODE 23    ; Reserved
ISR_NOERRCODE 24    ; Reserved
ISR_NOERRCODE 25    ; Reserved
ISR_NOERRCODE 26    ; Reserved
ISR_NOERRCODE 27    ; Reserved
ISR_NOERRCODE 28    ; Reserved
ISR_NOERRCODE 29    ; Reserved
ISR_ERRCODE   30    ; Security Exception (with error code)
ISR_NOERRCODE 31    ; Reserved

; Hardware IRQ handlers (IRQ 0-15, remapped to interrupts 32-47)
IRQ 0,  32      ; Programmable Interval Timer
IRQ 1,  33      ; Keyboard
IRQ 2,  34      ; Cascade (never raised)
IRQ 3,  35      ; COM2
IRQ 4,  36      ; COM1
IRQ 5,  37      ; LPT2
IRQ 6,  38      ; Floppy Disk
IRQ 7,  39      ; LPT1 (Spurious)
IRQ 8,  40      ; CMOS Real-Time Clock
IRQ 9,  41      ; Free for peripherals
IRQ 10, 42      ; Free for peripherals
IRQ 11, 43      ; Free for peripherals
IRQ 12, 44      ; PS2 Mouse
IRQ 13, 45      ; FPU / Coprocessor
IRQ 14, 46      ; Primary ATA Hard Disk
IRQ 15, 47      ; Secondary ATA Hard Disk

; Assembly function to load IDT
global idt_flush
idt_flush:
    mov eax, [esp + 4]      ; Get pointer to IDT pointer structure
    lidt [eax]              ; Load IDT
    ret
