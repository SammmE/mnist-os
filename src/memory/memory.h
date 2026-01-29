#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define PAGE_SIZE 4096

// Memory map addresses (set by bootloader)
#define MEMORY_MAP_ADDR 0x0500
#define MEMORY_MAP_COUNT_ADDR 0x04FC

// E820 memory map entry structure
typedef struct {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t acpi;
} __attribute__((packed)) e820_entry_t;

// E820 memory types
#define E820_USABLE         1
#define E820_RESERVED       2
#define E820_ACPI_RECLAIMABLE 3
#define E820_ACPI_NVS       4
#define E820_BAD_MEMORY     5

// Physical Memory Manager
void pmm_init(void);
void* pmm_alloc_page(void);
void pmm_free_page(void* ptr);
uint32_t pmm_get_free_pages(void);
uint32_t pmm_get_total_pages(void);

// Heap Allocator
void heap_init(uint32_t start, uint32_t size);
void* kmalloc(uint32_t size);
void kfree(void* ptr);
void* kmalloc_aligned(uint32_t size, uint32_t alignment);

// Complete memory initialization (PMM + Heap)
void memory_init(void);

#endif
