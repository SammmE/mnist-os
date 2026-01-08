#include "types.h"
// #include "weights.h"

#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define WHITE_ON_BLACK 0x0F

int cursor_col = 0;
int cursor_row = 0;

// Forward declarations (so _start knows these exist)
void vga_clear_screen();
void vga_print(char *message);
void enable_fpu();
void vga_set_cursor(int x, int y);

void _start() {
  cursor_col = 0;
  cursor_row = 0;

  vga_clear_screen();
  vga_set_cursor(0, 0);

  vga_print("Kernel started.");

  enable_fpu();
  vga_print("FPU Enabled.");

  while (1) {
  }
}

void enable_fpu() {
  __asm__ volatile(
      "mov %%cr0, %%eax\n"
      "and $0xFFFFFFFB, %%eax\n" // Clear EM (bit 2)
      "or $0x00000002, %%eax\n"  // Set MP (bit 1)
      "mov %%eax, %%cr0\n"
      "mov %%cr4, %%eax\n"
      "or $0x00000600, %%eax\n" // Set OSFXSR (bit 9) and OSXMMEXCPT (bit 10)
      "mov %%eax, %%cr4\n"
      :
      :
      : "eax");
}

void vga_clear_screen() {
  volatile uint16_t *video_memory = (volatile uint16_t *)VGA_ADDRESS;
  for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
    video_memory[i] = (uint16_t)WHITE_ON_BLACK << 8 | ' ';
  }
}

void vga_print_at(char *message, int col, int row) {
  volatile uint8_t *video_memory = (volatile uint8_t *)VGA_ADDRESS;
  if (col < 0 || col >= VGA_WIDTH || row < 0 || row >= VGA_HEIGHT) {
    return;
  }

  int offset = (row * VGA_WIDTH + col) * 2;
  int i = 0;
  while (message[i] != 0) {
    video_memory[offset] = message[i];
    video_memory[offset + 1] = WHITE_ON_BLACK;
    offset += 2;
    i++;
  }
}

void vga_print(char *message) {
  // Logic to wrap text
  if (cursor_row >= VGA_HEIGHT) {
    cursor_row = 0;
    cursor_col = 0;
  }

  vga_print_at(message, cursor_col, cursor_row);

  // Update cursor position based on string length
  int i = 0;
  while (message[i] != 0) {
    cursor_col++;
    if (cursor_col >= VGA_WIDTH) {
      cursor_col = 0;
      cursor_row++;
    }
    i++;
  }
  cursor_row++; // Newline at end of print
  cursor_col = 0;
}

void outb(uint16_t port, uint8_t val) {
  __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

void vga_set_cursor(int x, int y) {
  uint16_t pos = y * VGA_WIDTH + x;

  outb(0x3D4, 0x0F);
  outb(0x3D5, (uint8_t)(pos & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}