#include "disk.h"
#include "fat32.h"
#include "types.h"
#include "vga.h"

void kernel_main() {
  clear_screen();

  uint8_t buffer[512];

  println_string("Booting MNIST-OS");
  println_string("\nReading LBA 0... ");

  ata_read_sector(0, buffer);

  print_hex_byte(buffer[510]);
  print_char(' ');
  println_hex_byte(buffer[511]);

  if (buffer[510] == 0x55 && buffer[511] == 0xAA) {
    println_string("\n[SUCCESS] ATA PIO Driver Operational.");
  } else {
    println_string("\n[ERROR] Disk read mismatch.");
  }

  fat32_init();

  struct DirectoryEntry test_file;

  if (fat32_find_file("HELLO   TXT", &test_file)) {
    print_string("\nFound file! Size: ");
    print_hex(test_file.size);

    uint8_t file_data[8192];
    fat32_read_file(&test_file, file_data);

    print_string("\nFirst character of file: ");
    print_char(file_data[0]);
  }
}
