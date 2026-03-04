#include "disk.h"
#include "types.h"
#include "vga.h"

void kernel_main() {
  clear_screen();

  uint8_t buffer[512];

  print_string("Booting MNIST-OS...");
  print_string("\nReading LBA 0... ");

  ata_read_sector(0, buffer);

  print_hex_byte(buffer[510]);
  print_char(' ');
  print_hex_byte(buffer[511]);

  if (buffer[510] == 0x55 && buffer[511] == 0xAA) {
    print_string("\n[SUCCESS] ATA PIO Driver Operational.");
  } else {
    print_string("\n[ERROR] Disk read mismatch.");
  }
}
