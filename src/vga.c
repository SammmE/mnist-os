#include "vga.h"
#include "types.h"

static int cursor_offset = 0;

void clear_screen() {
  unsigned char *VGA = (unsigned char *)VGA_ADDRESS;

  for (int i = 0; i < 80 * 25 * 2; i++) {
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

void println_string(const char *str) {
  print_string(str);
  print_char('\n');
}

void print_hex_byte(uint8_t byte) {
  char *hex_digits = "0123456789ABCDEF";

  print_string("0x");

  uint8_t high = (byte >> 4) & 0x0F;
  uint8_t low = byte & 0x0F;

  print_char(hex_digits[high]);
  print_char(hex_digits[low]);
}

void println_hex_byte(uint8_t byte) {
  print_hex_byte(byte);
  print_char('\n');
}

void print_hex(uint32_t value) {
  char *hex_digits = "0123456789ABCDEF";

  print_string("0x");

  for (int i = 28; i >= 0; i -= 4) {
    uint8_t nibble = (value >> i) & 0x0F;
    print_char(hex_digits[nibble]);
  }
}

void println_hex(uint32_t value) {
  print_hex(value);
  print_char('\n');
}

void print_image(const float *image) {
  const char *density = " .:-=+*#%@@@@@@@";

  for (int i = 0; i < 64; i++) {
    int val = (int)image[i];

    if (val < 0)
      val = 0;
    if (val > 15)
      val = 15;

    print_char(density[val]);
    print_char(density[val]);

    if ((i & 7) == 7) {
      print_char('\n');
    }
  }
}

void println_image(const float *image) {
  print_image(image);
  print_char('\n');
}
