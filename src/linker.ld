ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS {
    /* Set exactly where the kernel should be loaded in memory */
    . = 0x1000;

    .text : {
        /* Ensure kernel_entry is first */
        src/kernel_entry.o(.text)
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
        *(COMMON)
    }
}
