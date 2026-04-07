#include "fat32.h"
#include "disk.h"
#include "vga.h"

uint32_t fat32_partition_start_lba;
uint16_t fat32_bytes_per_sector;
uint8_t fat32_sectors_per_cluster;
uint16_t fat32_reserved_sectors;
uint8_t fat32_fat_count;
uint32_t fat32_sectors_per_fat;
uint32_t fat32_root_cluster;

void fat32_init() {
  uint8_t buffer[512];

  ata_read_sector(0, buffer);

  struct PartitionEntry *part1 = (struct PartitionEntry *)(buffer + 0x1BE);

  fat32_partition_start_lba = part1->lba_begin;

  ata_read_sector(fat32_partition_start_lba, buffer);

  print_string("FAT32 Partition starts at LBA: ");
  println_hex(fat32_partition_start_lba);

  struct BPB *bpb = (struct BPB *)buffer;

  fat32_bytes_per_sector = bpb->bytes_per_sector;
  fat32_sectors_per_cluster = bpb->sectors_per_cluster;
  fat32_reserved_sectors = bpb->reserved_sectors;
  fat32_fat_count = bpb->fat_count;
  fat32_sectors_per_fat = bpb->sectors_per_fat_32;
  fat32_root_cluster = bpb->root_cluster;

  print_string("Bytes per sector: ");
  println_hex(fat32_bytes_per_sector);
  print_string("\nSectors per cluster: ");
  println_hex_byte(fat32_sectors_per_cluster);
}

uint32_t cluster_to_lba(uint32_t cluster) {
  return fat32_partition_start_lba + fat32_reserved_sectors +
         fat32_fat_count * fat32_sectors_per_fat +
         ((cluster - 2) * fat32_sectors_per_cluster);
}

int str_cmp_fat(const char *target, const uint8_t *disk_name) {
  for (int i = 0; i < 11; i++) {
    if (target[i] != disk_name[i])
      return 0;
  }
  return 1;
}

int fat32_find_file(const char *filename, struct DirectoryEntry *out_entry) {
  uint8_t buffer[512];

  uint32_t current_cluster = fat32_root_cluster;

  for (int sector_offset = 0; sector_offset < fat32_sectors_per_cluster;
       sector_offset++) {
    uint32_t lba = cluster_to_lba(current_cluster) + sector_offset;
    ata_read_sector(lba, buffer);

    struct DirectoryEntry *dir = (struct DirectoryEntry *)buffer;

    // 512 bytes / 32 bytes per entry = 16 entries per sector
    for (int i = 0; i < 16; i++) {
      if (dir[i].name[0] == 0x00)
        return 0; // 0x00 = end of directory
      if (dir[i].name[0] == 0xE5)
        continue; // 0xE5 = deleted file

      // 0x0F = "Long File Name" fragment, not a real file entry.
      if ((dir[i].attributes & 0x0F) == 0x0F)
        continue;

      if (str_cmp_fat(filename, dir[i].name)) {
        *out_entry = dir[i];
        return 1; // found
      }
    }
  }
  return 0; // not found
}

uint32_t fat32_get_next_cluster(uint32_t current_cluster) {
  uint8_t buffer[512];

  uint32_t fat_offset = current_cluster * 4;

  uint32_t fat_sector =
      fat32_partition_start_lba + fat32_reserved_sectors + (fat_offset / 512);

  uint32_t entry_offset = fat_offset % 512;

  ata_read_sector(fat_sector, buffer);

  uint32_t *table_ptr = (uint32_t *)(buffer + entry_offset);

  return (*table_ptr) & 0x0FFFFFFF;
}

void fat32_read_file(struct DirectoryEntry *file, uint8_t *out_buffer) {
  uint32_t current_cluster = (file->cluster_high << 16) | file->cluster_low;
  uint8_t *write_ptr = out_buffer;
  uint32_t bytes_remaining = file->size;
  uint8_t sector_buffer[512];

  // Loop until 0x0FFFFFF8 or higher
  while (current_cluster < 0x0FFFFFF8 && bytes_remaining > 0) {
    uint32_t lba = cluster_to_lba(current_cluster);

    // Read all sectors in cluster
    for (int i = 0; i < fat32_sectors_per_cluster; i++) {
      uint32_t chunk_size = bytes_remaining < 512 ? bytes_remaining : 512;

      ata_read_sector(lba + i, sector_buffer);
      for (uint32_t j = 0; j < chunk_size; j++) {
        write_ptr[j] = sector_buffer[j];
      }

      write_ptr += chunk_size;
      bytes_remaining -= chunk_size;
      if (bytes_remaining == 0) {
        break;
      }
    }

    current_cluster = fat32_get_next_cluster(current_cluster);
  }
}
