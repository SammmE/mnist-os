#include "timer.h"
#include "ports.h"
#include "screen.h"
#include "../isr.h"
#include "../idt.h"
#include <stdint.h>

// Global tick counter
static volatile uint32_t tick_count = 0;

// PIT constants
#define PIT_COMMAND 0x43
#define PIT_CHANNEL0 0x40
#define PIT_FREQUENCY 1193182  // Base PIT frequency in Hz

// Helper function to convert int to string
static void int_to_str(uint32_t num, char* str) {
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

// Timer interrupt handler
static void timer_callback(registers_t* regs) {
    (void)regs;  // Unused parameter
    tick_count++;
    
    // Display tick counter in top right corner every 100 ticks (~1 second at 100Hz)
    if (tick_count % 100 == 0) {
        char buffer[32];
        int_to_str(tick_count / 100, buffer);
        
        // Save current cursor position
        int old_offset = get_cursor_offset();
        
        // Print to top right corner (row 0, col 70)
        int offset = get_offset(0, 70);
        set_cursor_offset(offset);
        kprint("Time: ");
        kprint(buffer);
        kprint("s ");
        
        // Restore cursor position
        set_cursor_offset(old_offset);
    }
}

// Initialize the timer
void timer_init(uint32_t frequency) {
    // Register the timer interrupt handler
    register_interrupt_handler(IRQ0, timer_callback);
    
    // Calculate the divisor for the desired frequency
    uint32_t divisor = PIT_FREQUENCY / frequency;
    
    // Send command byte: Channel 0, lohi mode, rate generator
    port_byte_out(PIT_COMMAND, 0x36);
    
    // Send divisor (low byte, then high byte)
    uint8_t low = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);
    
    port_byte_out(PIT_CHANNEL0, low);
    port_byte_out(PIT_CHANNEL0, high);
    
    // Enable IRQ 0 (timer)
    irq_clear_mask(0);
}

// Get current tick count
uint32_t timer_get_ticks(void) {
    return tick_count;
}

// Wait for a specified number of ticks
void timer_wait(uint32_t ticks) {
    uint32_t end_tick = tick_count + ticks;
    while (tick_count < end_tick) {
        // Halt CPU until next interrupt for efficiency
        __asm__ volatile("hlt");
    }
}
