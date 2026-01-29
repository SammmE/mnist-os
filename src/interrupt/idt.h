#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// IDT entry structure (8 bytes)
typedef struct {
    uint16_t base_low;      // Lower 16 bits of handler address
    uint16_t selector;      // Kernel segment selector
    uint8_t  always0;       // Always 0
    uint8_t  flags;         // Type and attributes
    uint16_t base_high;     // Upper 16 bits of handler address
} __attribute__((packed)) idt_entry_t;

// IDT pointer structure for LIDT instruction
typedef struct {
    uint16_t limit;         // Size of IDT - 1
    uint32_t base;          // Base address of IDT
} __attribute__((packed)) idt_ptr_t;

// IDT constants
#define IDT_ENTRIES 256
#define KERNEL_CS 0x08      // Kernel code segment from GDT

// IDT gate type flags
#define IDT_FLAG_PRESENT    0x80    // Interrupt is present
#define IDT_FLAG_DPL0       0x00    // Descriptor privilege level 0 (kernel)
#define IDT_FLAG_DPL3       0x60    // Descriptor privilege level 3 (user)
#define IDT_FLAG_GATE_32BIT_INT  0x0E  // 32-bit interrupt gate
#define IDT_FLAG_GATE_32BIT_TRAP 0x0F  // 32-bit trap gate

// Standard IDT gate for kernel interrupts
#define IDT_GATE_KERNEL (IDT_FLAG_PRESENT | IDT_FLAG_DPL0 | IDT_FLAG_GATE_32BIT_INT)

// PIC (Programmable Interrupt Controller) constants
#define PIC1_COMMAND    0x20
#define PIC1_DATA       0x21
#define PIC2_COMMAND    0xA0
#define PIC2_DATA       0xA1
#define PIC_EOI         0x20    // End of Interrupt command

// IRQ numbers (remapped to interrupts 32-47)
#define IRQ0  32    // Programmable Interval Timer
#define IRQ1  33    // Keyboard
#define IRQ2  34    // Cascade (never raised)
#define IRQ3  35    // COM2
#define IRQ4  36    // COM1
#define IRQ5  37    // LPT2
#define IRQ6  38    // Floppy disk
#define IRQ7  39    // LPT1
#define IRQ8  40    // CMOS real-time clock
#define IRQ9  41    // Free for peripherals / legacy SCSI / NIC
#define IRQ10 42    // Free for peripherals / SCSI / NIC
#define IRQ11 43    // Free for peripherals / SCSI / NIC
#define IRQ12 44    // PS2 Mouse
#define IRQ13 45    // FPU / Coprocessor / Inter-processor
#define IRQ14 46    // Primary ATA Hard Disk
#define IRQ15 47    // Secondary ATA Hard Disk

// Function prototypes
void idt_init(void);
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags);
void pic_remap(void);
void pic_send_eoi(uint8_t irq);
void irq_set_mask(uint8_t irq);
void irq_clear_mask(uint8_t irq);

// Interrupt handler function type
typedef void (*isr_t)(void);

#endif
