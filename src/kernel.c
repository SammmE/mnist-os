#include "disk.h"
#include "fat32.h"
#include "types.h"
#include "vga.h"

void fpu_init() {
  uint32_t cr0;

  __asm__ __volatile__("mov %%cr0, %0" : "=r"(cr0));
  cr0 &= ~(1 << 2);
  cr0 |= (1 << 1);
  __asm__ __volatile__("mov %0, %%cr0" ::"r"(cr0));
  __asm__ __volatile__("fninit");
}

void kernel_main() {
  clear_screen();
  println_string("Booting MNIST-OS");

  fpu_init();
  fat32_init();

  struct DirectoryEntry test_file;

  if (fat32_find_file("HELLO   TXT", &test_file)) {
    print_string("Found file! Size: ");
    println_hex(test_file.size);

    uint8_t file_data[8192];
    fat32_read_file(&test_file, file_data);

    println_string((const char *)file_data);
  }

  float a = 6.7f;
  float b = 4.1f;
  float result = a * b;
  uint32_t int_result = (uint32_t)result;
  print_hex_byte(int_result);
}
