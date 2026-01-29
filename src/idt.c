#include "idt.h"
#include "isr.h"
#include "drivers/ports.h"
#include <stdint.h>

// IDT array (256 entries)
static idt_entry_t idt[IDT_ENTRIES];
static idt_ptr_t idt_ptr;

// Set an IDT gate
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = selector;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

// External assembly function to load IDT
extern void idt_flush(uint32_t);

// Remap the PIC (Programmable Interrupt Controller)
// The PIC uses IRQs 0-15, but they conflict with CPU exceptions 0-31
// So we remap them to interrupts 32-47
void pic_remap(void) {
    // Save masks
    uint8_t mask1 = port_byte_in(PIC1_DATA);
    uint8_t mask2 = port_byte_in(PIC2_DATA);
    
    // Start initialization sequence (in cascade mode)
    port_byte_out(PIC1_COMMAND, 0x11);
    port_byte_out(PIC2_COMMAND, 0x11);
    
    // Set vector offsets
    port_byte_out(PIC1_DATA, 0x20);  // Master PIC: IRQ 0-7  -> INT 32-39
    port_byte_out(PIC2_DATA, 0x28);  // Slave PIC:  IRQ 8-15 -> INT 40-47
    
    // Tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
    port_byte_out(PIC1_DATA, 0x04);
    // Tell Slave PIC its cascade identity (0000 0010)
    port_byte_out(PIC2_DATA, 0x02);
    
    // Set mode to 8086
    port_byte_out(PIC1_DATA, 0x01);
    port_byte_out(PIC2_DATA, 0x01);
    
    // Restore saved masks
    port_byte_out(PIC1_DATA, mask1);
    port_byte_out(PIC2_DATA, mask2);
}

// Send End of Interrupt signal to PIC
void pic_send_eoi(uint8_t irq) {
    // If IRQ came from slave PIC (IRQ 8-15), send EOI to slave
    if (irq >= 8) {
        port_byte_out(PIC2_COMMAND, PIC_EOI);
    }
    // Always send EOI to master PIC
    port_byte_out(PIC1_COMMAND, PIC_EOI);
}

// Disable (mask) an IRQ line
void irq_set_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    value = port_byte_in(port) | (1 << irq);
    port_byte_out(port, value);
}

// Enable (unmask) an IRQ line
void irq_clear_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    value = port_byte_in(port) & ~(1 << irq);
    port_byte_out(port, value);
}

// Initialize the IDT
void idt_init(void) {
    // Set up IDT pointer
    idt_ptr.limit = sizeof(idt_entry_t) * IDT_ENTRIES - 1;
    idt_ptr.base = (uint32_t)&idt;
    
    // Clear IDT
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, 0, 0, 0);
    }
    
    // Remap the PIC
    pic_remap();
    
    // Install ISRs and IRQs
    isr_install();
    
    // Load the IDT
    idt_flush((uint32_t)&idt_ptr);
}
