#include "disk.h"
#include "fat32.h"
#include "types.h"
#include "vga.h"

void kernel_main() {
  clear_screen();

  uint8_t buffer[512];

  println_string("Booting MNIST-OS");

  fat32_init();

  struct DirectoryEntry test_file;

  if (fat32_find_file("HELLO   TXT", &test_file)) {
    print_string("Found file! Size: ");
    println_hex(test_file.size);

    uint8_t file_data[8192];
    fat32_read_file(&test_file, file_data);

    println_string((const char *)file_data);
  }
}
