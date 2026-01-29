#include "drivers/screen.h"
#include "drivers/timer.h"
#include "drivers/keyboard.h"
#include "memory.h"
#include "idt.h"

// Helper function to convert int to string
static void int_to_str(int num, char* str) {
    int i = 0;
    int is_negative = 0;
    
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }
    
    while (num > 0) {
        str[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    if (is_negative) {
        str[i++] = '-';
    }
    
    str[i] = '\0';
    
    // Reverse string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

void kmain() {
    clear_screen();
    
    // Initialize 
    idt_init();
    memory_init();
    timer_init(100);
    keyboard_init();
    __asm__ volatile("sti");
    
    while (1) {
        __asm__ volatile("hlt");
    }
}
