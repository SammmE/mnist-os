#include "keyboard.h"
#include "ports.h"
#include "screen.h"
#include "../isr.h"
#include "../idt.h"
#include <stdint.h>

// Keyboard data and control ports
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// Keyboard modifier states
static uint8_t shift_pressed = 0;
static uint8_t caps_lock = 0;
static uint8_t ctrl_pressed = 0;
static uint8_t alt_pressed = 0;

// Simple keyboard buffer
#define BUFFER_SIZE 256
static char keyboard_buffer[BUFFER_SIZE];
static volatile int buffer_read = 0;
static volatile int buffer_write = 0;

// US QWERTY scancode to ASCII translation table
// Index is the scancode, value is the ASCII character
static const char scancode_to_ascii[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',  // 0x00-0x0E
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',  // 0x0F-0x1C
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',          // 0x1D-0x29
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,            // 0x2A-0x36
    '*', 0, ' '                                                              // 0x37-0x39
};

// Shifted characters (when shift is pressed)
static const char scancode_to_ascii_shift[] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',  // 0x00-0x0E
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',  // 0x0F-0x1C
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',           // 0x1D-0x29
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,             // 0x2A-0x36
    '*', 0, ' '                                                              // 0x37-0x39
};

// Scancode constants
#define SC_LSHIFT_PRESS   0x2A
#define SC_LSHIFT_RELEASE 0xAA
#define SC_RSHIFT_PRESS   0x36
#define SC_RSHIFT_RELEASE 0xB6
#define SC_CAPS_LOCK      0x3A
#define SC_CTRL_PRESS     0x1D
#define SC_CTRL_RELEASE   0x9D
#define SC_ALT_PRESS      0x38
#define SC_ALT_RELEASE    0xB8
#define SC_MAX            0x39

// Add character to keyboard buffer
static void buffer_put(char c) {
    int next_write = (buffer_write + 1) % BUFFER_SIZE;
    if (next_write != buffer_read) {
        keyboard_buffer[buffer_write] = c;
        buffer_write = next_write;
    }
}

// Get character from keyboard buffer (non-blocking)
static int buffer_get(char* c) {
    if (buffer_read == buffer_write) {
        return 0;  // Buffer empty
    }
    *c = keyboard_buffer[buffer_read];
    buffer_read = (buffer_read + 1) % BUFFER_SIZE;
    return 1;  // Character available
}

// Keyboard interrupt handler
static void keyboard_callback(registers_t* regs) {
    (void)regs;  // Unused parameter
    
    // Read scancode from keyboard controller
    uint8_t scancode = port_byte_in(KEYBOARD_DATA_PORT);
    
    // Handle key releases (scancode with bit 7 set)
    if (scancode & 0x80) {
        // Key release
        switch (scancode) {
            case SC_LSHIFT_RELEASE:
            case SC_RSHIFT_RELEASE:
                shift_pressed = 0;
                break;
            case SC_CTRL_RELEASE:
                ctrl_pressed = 0;
                break;
            case SC_ALT_RELEASE:
                alt_pressed = 0;
                break;
        }
        return;
    }
    
    // Handle key presses
    switch (scancode) {
        case SC_LSHIFT_PRESS:
        case SC_RSHIFT_PRESS:
            shift_pressed = 1;
            break;
            
        case SC_CAPS_LOCK:
            caps_lock = !caps_lock;
            break;
            
        case SC_CTRL_PRESS:
            ctrl_pressed = 1;
            break;
            
        case SC_ALT_PRESS:
            alt_pressed = 1;
            break;
            
        default:
            // Convert scancode to ASCII
            if (scancode <= SC_MAX) {
                char ascii;
                
                if (shift_pressed) {
                    ascii = scancode_to_ascii_shift[scancode];
                } else {
                    ascii = scancode_to_ascii[scancode];
                }
                
                // Apply caps lock to letters only
                if (caps_lock && ascii >= 'a' && ascii <= 'z') {
                    ascii -= 32;  // Convert to uppercase
                } else if (caps_lock && ascii >= 'A' && ascii <= 'Z' && !shift_pressed) {
                    // Caps lock is on but no shift - already uppercase from table
                } else if (caps_lock && ascii >= 'A' && ascii <= 'Z' && shift_pressed) {
                    ascii += 32;  // Caps + shift = lowercase
                }
                
                // Handle special key combinations
                if (ctrl_pressed) {
                    // Ctrl+C, Ctrl+D, etc. could be handled here
                    if (ascii == 'c' || ascii == 'C') {
                        kprint("^C");
                        return;
                    }
                }
                
                // If valid ASCII character, print it and buffer it
                if (ascii != 0) {
                    char str[2] = {ascii, '\0'};
                    kprint(str);
                    buffer_put(ascii);
                }
            }
            break;
    }
}

// Initialize the keyboard driver
void keyboard_init(void) {
    // Register keyboard interrupt handler (IRQ1)
    register_interrupt_handler(IRQ1, keyboard_callback);
    
    // Enable IRQ 1 (keyboard)
    irq_clear_mask(1);
    
    kprint("Keyboard initialized. Start typing!\n");
}

// Get a character from keyboard buffer (blocking)
char keyboard_getchar(void) {
    char c;
    while (!buffer_get(&c)) {
        // Wait for character - halt CPU for efficiency
        __asm__ volatile("hlt");
    }
    return c;
}

// Check if a key is available in buffer
int keyboard_available(void) {
    return buffer_read != buffer_write;
}
