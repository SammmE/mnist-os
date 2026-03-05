#ifndef FAT32_H
#define FAT32_H

#include "types.h"

struct PartitionEntry {
  uint8_t bootable;
  uint8_t start_chs[3];
  uint8_t partition_id;
  uint8_t end_chs[3];
  uint32_t lba_begin;
  uint32_t size_in_sectors;
} __attribute__((packed));

struct BPB {
  uint8_t jmp[3];
  uint8_t oem[8];

  uint16_t bytes_per_sector;
  uint8_t sectors_per_cluster;
  uint16_t reserved_sectors;
  uint8_t fat_count;
  uint16_t dir_entries;
  uint16_t total_sectors_16;
  uint8_t media_descriptor;
  uint16_t sectors_per_fat_16;
  int16_t sectors_per_track;
  uint16_t heads;
  uint32_t hidden_sectors;
  uint32_t total_sectors_32;

  uint32_t sectors_per_fat_32;
  uint16_t flags;
  uint16_t fat_version;
  uint32_t root_cluster;
  uint16_t fs_info_sector;
  uint16_t backup_boot_sector;
  uint8_t reserved[12];
  uint8_t drive_number;
  uint8_t reserved1;
  uint8_t boot_signature;
  uint32_t volume_id;
  uint8_t volume_label[11];
  uint8_t fs_type[8];
} __attribute__((packed));

struct DirectoryEntry {
  uint8_t name[11];
  uint8_t attributes;
  uint8_t reserved;
  uint8_t creation_time_tenths;
  uint16_t creation_time;
  uint16_t creation_date;
  uint16_t accessed_date;
  uint16_t cluster_high;
  uint16_t modification_time;
  uint16_t modification_date;
  uint16_t cluster_low;
  uint32_t size;
} __attribute__((packed));

void fat32_init();

uint32_t cluster_to_lba(uint32_t cluster);

void fat32_read_file(struct DirectoryEntry *file, uint8_t *out_buffer);
int fat32_find_file(const char *filename, struct DirectoryEntry *out_entry);

#endif // FAT32_H
