#include "memory.h"

static uint8_t* pmm_bitmap;
static uint32_t pmm_bitmap_size;
static uint32_t total_pages;
static uint32_t free_pages;
static uint32_t pmm_highest_page;

extern uint32_t _kernel_end;

// Bitmap manipulation
static inline void pmm_set_bit(uint32_t bit) {
    pmm_bitmap[bit / 8] |= (1 << (bit % 8));
}

static inline void pmm_clear_bit(uint32_t bit) {
    pmm_bitmap[bit / 8] &= ~(1 << (bit % 8));
}

static inline uint8_t pmm_test_bit(uint32_t bit) {
    return pmm_bitmap[bit / 8] & (1 << (bit % 8));
}

// Find first free page in bitmap
static uint32_t pmm_find_free_page(void) {
    for (uint32_t i = 0; i < total_pages; i++) {
        if (!pmm_test_bit(i)) {
            return i;
        }
    }
    return 0xFFFFFFFF;  // No free pages, we can mark them as free later
}

// mark as used
static void pmm_mark_region_used(uint32_t base, uint32_t size) {
    uint32_t start_page = base / PAGE_SIZE;
    uint32_t end_page = (base + size + PAGE_SIZE - 1) / PAGE_SIZE;
    
    for (uint32_t i = start_page; i < end_page && i < total_pages; i++) {
        if (!pmm_test_bit(i)) {
            pmm_set_bit(i);
            if (free_pages > 0) free_pages--;
        }
    }
}

// mark as free
static void pmm_mark_region_free(uint32_t base, uint32_t size) {
    uint32_t start_page = base / PAGE_SIZE;
    uint32_t end_page = (base + size + PAGE_SIZE - 1) / PAGE_SIZE;
    
    for (uint32_t i = start_page; i < end_page && i < total_pages; i++) {
        if (pmm_test_bit(i)) {
            pmm_clear_bit(i);
            free_pages++;
        }
    }
}

void pmm_init(void) {
    // Read memory map from bootloader
    uint32_t entry_count = *(uint32_t*)MEMORY_MAP_COUNT_ADDR;
    e820_entry_t* entries = (e820_entry_t*)MEMORY_MAP_ADDR;
    
    // Find highest usable memory address
    uint64_t highest_addr = 0;
    for (uint32_t i = 0; i < entry_count; i++) {
        if (entries[i].type == E820_USABLE) {
            uint64_t end = entries[i].base + entries[i].length;
            if (end > highest_addr) {
                highest_addr = end;
            }
        }
    }
    
    // Calculate total pages (limit to 4GB for 32-bit)
    if (highest_addr > 0xFFFFFFFF) {
        highest_addr = 0xFFFFFFFF;
    }
    total_pages = highest_addr / PAGE_SIZE;
    pmm_highest_page = total_pages;
    
    // Calculate bitmap size (1 bit per page)
    pmm_bitmap_size = (total_pages + 7) / 8;
    
    // Place bitmap right after kernel
    uint32_t kernel_end = (uint32_t)&_kernel_end;
    kernel_end = (kernel_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);  // Align to page
    pmm_bitmap = (uint8_t*)kernel_end;
    
    // Initialize bitmap - mark all as used
    for (uint32_t i = 0; i < pmm_bitmap_size; i++) {
        pmm_bitmap[i] = 0xFF;
    }
    free_pages = 0;
    
    // Mark usable regions as free based on E820 map
    for (uint32_t i = 0; i < entry_count; i++) {
        if (entries[i].type == E820_USABLE) {
            // Only consider memory within 32 bit range
            if (entries[i].base < 0xFFFFFFFF) {
                uint32_t base = (uint32_t)entries[i].base;
                uint32_t length = (uint32_t)entries[i].length;
                
                if ((uint64_t)base + length > 0xFFFFFFFF) {
                    length = 0xFFFFFFFF - base;
                }
                
                pmm_mark_region_free(base, length);
            }
        }
    }
    
    // Mark reserved regions as used
    // 1. First 1MB
    pmm_mark_region_used(0x0, 0x100000);
    
    // 2. Kernel region (from 0x1000 to end of kernel)
    pmm_mark_region_used(0x1000, kernel_end - 0x1000);
    
    // 3. Bitmap
    pmm_mark_region_used((uint32_t)pmm_bitmap, pmm_bitmap_size);
}

void* pmm_alloc_page(void) {
    if (free_pages == 0) {
        return 0;  // Out of memory
    }
    
    uint32_t page = pmm_find_free_page();
    if (page == 0xFFFFFFFF) {
        return 0;  // No free page found
    }
    
    pmm_set_bit(page);
    free_pages--;
    
    return (void*)(page * PAGE_SIZE);
}

void pmm_free_page(void* ptr) {
    uint32_t addr = (uint32_t)ptr;
    if (addr % PAGE_SIZE != 0) {
        return;  // Not page-aligned
    }
    
    uint32_t page = addr / PAGE_SIZE;
    if (page >= total_pages) {
        return;  // Invalid page
    }
    
    if (pmm_test_bit(page)) {
        pmm_clear_bit(page);
        free_pages++;
    }
}

uint32_t pmm_get_free_pages(void) {
    return free_pages;
}

uint32_t pmm_get_total_pages(void) {
    return total_pages;
}

typedef struct heap_block {
    uint32_t size;              // Size of usable area (excluding header)
    uint8_t is_free;            // 1 = free, 0 = allocated
    struct heap_block* next;
    struct heap_block* prev;
} heap_block_t;

#define HEAP_BLOCK_HEADER_SIZE sizeof(heap_block_t)
#define MIN_ALLOC_SIZE 16

static heap_block_t* heap_start = 0;
static uint32_t heap_size = 0;

void heap_init(uint32_t start, uint32_t size) {
    // Align start to 8-byte boundary
    start = (start + 7) & ~7;
    
    heap_start = (heap_block_t*)start;
    heap_size = size;
    
    // Create initial free block
    heap_start->size = size - HEAP_BLOCK_HEADER_SIZE;
    heap_start->is_free = 1;
    heap_start->next = 0;
    heap_start->prev = 0;
}

// Try to coalesce with neighboring free blocks
static void heap_coalesce(heap_block_t* block) {
    // Merge with next block if it's free
    if (block->next && block->next->is_free) {
        block->size += HEAP_BLOCK_HEADER_SIZE + block->next->size;
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }
    }
    
    // Merge with previous block if it's free
    if (block->prev && block->prev->is_free) {
        block->prev->size += HEAP_BLOCK_HEADER_SIZE + block->size;
        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
    }
}

void* kmalloc(uint32_t size) {
    if (!heap_start || size == 0) {
        return 0;
    }
    
    // Align size to 8 bytes
    size = (size + 7) & ~7;
    
    // Ensure minimum allocation size
    if (size < MIN_ALLOC_SIZE) {
        size = MIN_ALLOC_SIZE;
    }
    
    // Find first-fit free block
    heap_block_t* current = heap_start;
    while (current) {
        if (current->is_free && current->size >= size) {
            // Split block if there's enough space for another block
            if (current->size >= size + HEAP_BLOCK_HEADER_SIZE + MIN_ALLOC_SIZE) {
                heap_block_t* new_block = (heap_block_t*)((uint8_t*)current + HEAP_BLOCK_HEADER_SIZE + size);
                new_block->size = current->size - size - HEAP_BLOCK_HEADER_SIZE;
                new_block->is_free = 1;
                new_block->next = current->next;
                new_block->prev = current;
                
                if (current->next) {
                    current->next->prev = new_block;
                }
                current->next = new_block;
                current->size = size;
            }
            
            current->is_free = 0;
            return (void*)((uint8_t*)current + HEAP_BLOCK_HEADER_SIZE);
        }
        current = current->next;
    }
    
    return 0;  // No suitable block found
}

void kfree(void* ptr) {
    if (!ptr || !heap_start) {
        return;
    }
    
    // Get block header
    heap_block_t* block = (heap_block_t*)((uint8_t*)ptr - HEAP_BLOCK_HEADER_SIZE);
    
    // Validate block is within heap
    if ((uint32_t)block < (uint32_t)heap_start || 
        (uint32_t)block >= (uint32_t)heap_start + heap_size) {
        return;
    }
    
    block->is_free = 1;
    heap_coalesce(block);
}

void* kmalloc_aligned(uint32_t size, uint32_t alignment) {
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        return 0;  // Alignment must be power of 2
    }
    
    // Allocate extra space for alignment adjustment
    uint32_t total_size = size + alignment + HEAP_BLOCK_HEADER_SIZE;
    void* ptr = kmalloc(total_size);
    if (!ptr) {
        return 0;
    }
    
    // Calculate aligned address
    uint32_t addr = (uint32_t)ptr;
    uint32_t aligned_addr = (addr + alignment - 1) & ~(alignment - 1);
    
    // If already aligned, return as-is
    if (aligned_addr == addr) {
        return ptr;
    }
    
    return ptr;
}

void memory_init(void) {
    // Initialize Physical Memory Manager
    pmm_init();
    
    // Calculate heap start (after bitmap)
    uint32_t kernel_end = (uint32_t)&_kernel_end;
    kernel_end = (kernel_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    uint32_t heap_start_addr = kernel_end + pmm_bitmap_size;
    heap_start_addr = (heap_start_addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    
    // Allocate 1MB for heap
    uint32_t heap_sz = 1024 * 1024;
    heap_init(heap_start_addr, heap_sz);
    
    // Mark heap region as used in PMM
    pmm_mark_region_used(heap_start_addr, heap_sz);
}
