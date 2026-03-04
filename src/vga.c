#include "vga.h"
#include "types.h"

static int cursor_offset = 0;

void clear_screen() {
  unsigned char *VGA = (unsigned char *)VGA_ADDRESS;

  for (int i = 0; i < 320 * 200; i++) {
    VGA[i] = 0;
  }

  cursor_offset = 0;
}

void print_char(char c) {
  unsigned char *VGA = (unsigned char *)VGA_ADDRESS;

  if (c == '\n') {
    int current_row = cursor_offset / 160;
    cursor_offset = (current_row + 1) * 160;
  } else {
    VGA[cursor_offset] = c;
    VGA[cursor_offset + 1] = WHITE_ON_BLACK;
    cursor_offset += 2;
  }

  if (cursor_offset >= 160 * 25) {
    cursor_offset = 0;
  }
}

void print_string(const char *str) {
  while (*str != '\0') {
    print_char(*str++);
  }
}

void print_hex_byte(uint8_t byte) {
  char *hex_digits = "0123456789ABCDEF";

  print_string("0x");

  uint8_t high = (byte >> 4) & 0x0F;
  uint8_t low = byte & 0x0F;

  print_char(hex_digits[high]);
  print_char(hex_digits[low]);
}

void print_hex(uint32_t value) {
  char *hex_digits = "0123456789ABCDEF";

  print_string("0x");

  for (int i = 28; i >= 0; i -= 4) {
    uint8_t nibble = (value >> i) & 0x0F;
    print_char(hex_digits[nibble]);
  }
}
