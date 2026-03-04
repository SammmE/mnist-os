#include "disk.h"
#include "ports.h"
#include "types.h"

void ata_read_sector(uint32_t lba, uint8_t *buffer) {
  port_byte_out(ATA_DRIVE_PORT, 0xE0 | ((lba >> 24) & 0x0F));
  port_byte_out(ATA_SECTOR_COUNT_PORT, 1);

  port_byte_out(ATA_LBA_LOW_PORT, lba & 0xFF);
  port_byte_out(ATA_LBA_MID_PORT, ((lba >> 8) & 0xFF));
  port_byte_out(ATA_LBA_HIGH_PORT, ((lba >> 16) & 0xFF));

  port_byte_out(ATA_COMMAND_PORT, 0x20);
  ata_wait_ready();

  uint16_t *target = (uint16_t *)buffer;

  for (int i = 0; i < 256; i++) {
    target[i] = port_word_in(ATA_DATA_PORT);
  }
}

void ata_wait_ready(void) {
  uint8_t res;
  do {
    res = port_byte_in(ATA_STATUS_PORT);
  } while (!(!((res >> 7) & 1) && (res >> 3) & 1));
}
