#include "isr.h"
#include "idt.h"
#include "drivers/screen.h"
#include <stdint.h>

// Array of interrupt handler function pointers
static interrupt_handler_t interrupt_handlers[256] = {0};

// Exception messages
static const char* exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Security Exception",
    "Reserved"
};

// Helper function to convert int to hex string
static void int_to_hex(uint32_t num, char* str) {
    str[0] = '0';
    str[1] = 'x';
    
    for (int i = 9; i >= 2; i--) {
        uint8_t digit = num & 0xF;
        str[i] = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
        num >>= 4;
    }
    str[10] = '\0';
}

// Helper function to convert int to decimal string
static void int_to_dec(uint32_t num, char* str) {
    int i = 0;
    
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    
    while (num > 0) {
        str[i++] = '0' + (num % 10);
        num /= 10;
    }
    str[i] = '\0';
    
    // Reverse string
    for (int j = 0; j < i / 2; j++) {
        char temp = str[j];
        str[j] = str[i - j - 1];
        str[i - j - 1] = temp;
    }
}

// Register a custom interrupt handler
void register_interrupt_handler(uint8_t n, interrupt_handler_t handler) {
    interrupt_handlers[n] = handler;
}

// Main ISR handler (called from assembly stub)
void isr_handler(registers_t* regs) {
    char buffer[32];
    
    // Check if we have a custom handler for this interrupt
    if (interrupt_handlers[regs->int_no] != 0) {
        interrupt_handler_t handler = interrupt_handlers[regs->int_no];
        handler(regs);
    } else {
        // No custom handler - this is a CPU exception, halt the system
        kprint("\n!!! EXCEPTION: ");
        kprint(exception_messages[regs->int_no]);
        kprint(" !!!\n");
        
        kprint("Interrupt Number: ");
        int_to_dec(regs->int_no, buffer);
        kprint(buffer);
        kprint("\n");
        
        kprint("Error Code: ");
        int_to_hex(regs->err_code, buffer);
        kprint(buffer);
        kprint("\n");
        
        kprint("\nRegisters:\n");
        kprint("EAX=");
        int_to_hex(regs->eax, buffer);
        kprint(buffer);
        kprint(" EBX=");
        int_to_hex(regs->ebx, buffer);
        kprint(buffer);
        kprint(" ECX=");
        int_to_hex(regs->ecx, buffer);
        kprint(buffer);
        kprint(" EDX=");
        int_to_hex(regs->edx, buffer);
        kprint(buffer);
        kprint("\n");
        
        kprint("ESI=");
        int_to_hex(regs->esi, buffer);
        kprint(buffer);
        kprint(" EDI=");
        int_to_hex(regs->edi, buffer);
        kprint(buffer);
        kprint(" EBP=");
        int_to_hex(regs->ebp, buffer);
        kprint(buffer);
        kprint(" ESP=");
        int_to_hex(regs->esp, buffer);
        kprint(buffer);
        kprint("\n");
        
        kprint("EIP=");
        int_to_hex(regs->eip, buffer);
        kprint(buffer);
        kprint(" CS=");
        int_to_hex(regs->cs, buffer);
        kprint(buffer);
        kprint(" EFLAGS=");
        int_to_hex(regs->eflags, buffer);
        kprint(buffer);
        kprint("\n");
        
        kprint("\nSystem Halted.\n");
        
        // Halt the system
        for (;;) {
            __asm__ volatile("hlt");
        }
    }
}

// Main IRQ handler (called from assembly stub)
void irq_handler(registers_t* regs) {
    // Send End of Interrupt signal to PIC
    pic_send_eoi(regs->int_no - 32);
    
    // Check if we have a custom handler for this IRQ
    if (interrupt_handlers[regs->int_no] != 0) {
        interrupt_handler_t handler = interrupt_handlers[regs->int_no];
        handler(regs);
    }
}

// Install all ISRs and IRQs into the IDT
void isr_install(void) {
    // Install CPU exception handlers (ISRs 0-31)
    idt_set_gate(0, (uint32_t)isr0, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(1, (uint32_t)isr1, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(2, (uint32_t)isr2, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(3, (uint32_t)isr3, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(4, (uint32_t)isr4, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(5, (uint32_t)isr5, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(6, (uint32_t)isr6, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(7, (uint32_t)isr7, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(8, (uint32_t)isr8, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(9, (uint32_t)isr9, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(10, (uint32_t)isr10, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(11, (uint32_t)isr11, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(12, (uint32_t)isr12, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(13, (uint32_t)isr13, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(14, (uint32_t)isr14, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(15, (uint32_t)isr15, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(16, (uint32_t)isr16, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(17, (uint32_t)isr17, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(18, (uint32_t)isr18, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(19, (uint32_t)isr19, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(20, (uint32_t)isr20, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(21, (uint32_t)isr21, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(22, (uint32_t)isr22, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(23, (uint32_t)isr23, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(24, (uint32_t)isr24, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(25, (uint32_t)isr25, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(26, (uint32_t)isr26, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(27, (uint32_t)isr27, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(28, (uint32_t)isr28, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(29, (uint32_t)isr29, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(30, (uint32_t)isr30, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(31, (uint32_t)isr31, KERNEL_CS, IDT_GATE_KERNEL);
    
    // Install hardware interrupt handlers (IRQs 0-15 -> interrupts 32-47)
    idt_set_gate(32, (uint32_t)irq0, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(33, (uint32_t)irq1, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(34, (uint32_t)irq2, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(35, (uint32_t)irq3, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(36, (uint32_t)irq4, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(37, (uint32_t)irq5, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(38, (uint32_t)irq6, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(39, (uint32_t)irq7, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(40, (uint32_t)irq8, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(41, (uint32_t)irq9, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(42, (uint32_t)irq10, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(43, (uint32_t)irq11, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(44, (uint32_t)irq12, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(45, (uint32_t)irq13, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(46, (uint32_t)irq14, KERNEL_CS, IDT_GATE_KERNEL);
    idt_set_gate(47, (uint32_t)irq15, KERNEL_CS, IDT_GATE_KERNEL);
}
