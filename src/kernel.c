#include "drivers/screen.h"

void kmain() {
    clear_screen();
    kprint_at("Hello, World! Welcome to MNIST-OS", 10, 5);
    kprint("\nThis text is printed on the next line.");
    kprint("\nAnd this wraps around if it gets too long to fit on one single line in the screen buffer.");
}
