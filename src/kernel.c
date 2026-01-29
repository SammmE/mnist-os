#include "drivers/screen.h"
#include "memory.h"

// Helper function to convert int to string
static void int_to_str(int num, char* str) {
    int i = 0;
    int is_negative = 0;
    
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    
    if (num < 0) {
        is_negative = 1;
        num = -num;
    }
    
    while (num > 0) {
        str[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    if (is_negative) {
        str[i++] = '-';
    }
    
    str[i] = '\0';
    
    // Reverse string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

void kmain() {
    clear_screen();
    memory_init();
    
    // Display memory statistics
    char buffer[32];
    kprint("Memory Manager Initialized!\n");
    
    int_to_str(pmm_get_total_pages(), buffer);
    kprint("Total pages: ");
    kprint(buffer);
    kprint("\n");
    
    int_to_str(pmm_get_free_pages(), buffer);
    kprint("Free pages: ");
    kprint(buffer);
    kprint("\n");
    
    int_to_str((pmm_get_free_pages() * 4), buffer);
    kprint("Free memory: ");
    kprint(buffer);
    kprint(" KB\n\n");
    
    kprint("Testing Physical Memory Manager...\n");
    void* page1 = pmm_alloc_page();
    void* page2 = pmm_alloc_page();
    void* page3 = pmm_alloc_page();
    
    kprint("Allocated 3 pages: ");
    int_to_str((int)page1, buffer);
    kprint(buffer);
    kprint(", ");
    int_to_str((int)page2, buffer);
    kprint(buffer);
    kprint(", ");
    int_to_str((int)page3, buffer);
    kprint(buffer);
    kprint("\n");
    
    pmm_free_page(page2);
    kprint("Freed middle page\n");
    
    int_to_str(pmm_get_free_pages(), buffer);
    kprint("Free pages now: ");
    kprint(buffer);
    kprint("\n\n");
    
    kprint("Testing Heap Allocator...\n");
    
    int* array1 = (int*)kmalloc(10 * sizeof(int));
    int* array2 = (int*)kmalloc(20 * sizeof(int));
    char* str = (char*)kmalloc(50);
    
    if (array1) {
        kprint("Allocated array1 (10 ints)\n");
        array1[0] = 42;
        array1[9] = 99;
    }
    
    if (array2) {
        kprint("Allocated array2 (20 ints)\n");
    }
    
    if (str) {
        kprint("Allocated string buffer (50 bytes)\n");
        str[0] = 'H';
        str[1] = 'i';
        str[2] = '!';
        str[3] = '\0';
    }
    
    kprint("\nTest data in array1[0]: ");
    int_to_str(array1[0], buffer);
    kprint(buffer);
    kprint("\n");
    
    kprint("Test data in array1[9]: ");
    int_to_str(array1[9], buffer);
    kprint(buffer);
    kprint("\n");
    
    kprint("Test string: ");
    kprint(str);
    kprint("\n\n");
    
    // Test freeing and coalescing
    kprint("Freeing allocations...\n");
    kfree(array1);
    kfree(str);
    kfree(array2);
    kprint("All heap blocks freed (coalesced)\n\n");
    
    kprint("Memory subsystem tests complete!\n");
}
