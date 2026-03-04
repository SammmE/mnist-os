#include "disk.h"
#include "types.h"

#define VGA_ADDRESS 0xB8000
#define WHITE_ON_BLACK 0x0F

static int cursor_offset = 0;

void print_char(char c) {
  char *video_memory = (char *)VGA_ADDRESS;
  video_memory[cursor_offset] = c;
  video_memory[cursor_offset + 1] = WHITE_ON_BLACK;
  cursor_offset += 2;
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

void kernel_main() {
  uint8_t buffer[512];

  ata_read_sector(0, buffer);

  if (buffer[510] == 0x55 && buffer[511] == 0xAA) {
    print_string("Read disk!");
  }

  print_hex_byte(buffer[510] == 0x55);
  print_hex_byte(buffer[511] == 0x55);
}
