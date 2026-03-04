#ifndef DISK_H
#define DISK_H

#include "types.h"

#define ATA_DATA_PORT 0x1F0
#define ATA_SECTOR_COUNT_PORT 0x1F2
#define ATA_LBA_LOW_PORT 0x1F3
#define ATA_LBA_MID_PORT 0x1F4
#define ATA_LBA_HIGH_PORT 0x1F5
#define ATA_DRIVE_PORT 0x1F6
#define ATA_COMMAND_PORT 0x1F7
#define ATA_STATUS_PORT 0x1F7

void ata_read_sector(uint32_t lba, uint8_t *buffer);

void ata_wait_ready(void);

#endif // DISK_H
