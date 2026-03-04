#include "disk.h"
#include "types.h"

#define VGA_ADDRESS 0xB8000
#define WHITE_ON_BLACK 0x0F

void print_string(const char *str) {
  char *video_memory = (char *)VGA_ADDRESS;
  int offset = 0;
  while (*str != '\0') {
    video_memory[offset] = *str;
    video_memory[offset + 1] = WHITE_ON_BLACK;
    str++;
    offset += 2;
  }
}

void kernel_main() {
  uint8_t buffer[512];

  ata_read_sector(0, buffer);

  if (buffer[510] == 0x55 && buffer[511] == 0xAA) {
    print_string("Read disk!");
  }
}
