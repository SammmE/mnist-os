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
  print_string(
      "Phase 1: Successfully executing C Kernel in 32-bit Protected Mode!");
}
