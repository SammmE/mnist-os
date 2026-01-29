# Ensure these point to your actual cross-compiler binaries
CC = gcc
LD = ld
ASM = nasm

SRC_DIR = src
BUILD_DIR = build

CFLAGS = -ffreestanding -m32 -g -c -I$(SRC_DIR) -fno-pie -fno-pic -fno-stack-protector

all: $(BUILD_DIR)/os-image.bin

$(BUILD_DIR)/boot.bin: $(SRC_DIR)/boot/boot.asm
	@mkdir -p $(BUILD_DIR)
	$(ASM) -f bin $< -o $@

$(BUILD_DIR)/k_entry.o: $(SRC_DIR)/kernel/k_entry.asm
	@mkdir -p $(BUILD_DIR)
	$(ASM) -f elf $< -o $@

$(BUILD_DIR)/isr.o: $(SRC_DIR)/interrupt/isr.asm
	@mkdir -p $(BUILD_DIR)
	$(ASM) -f elf $< -o $@

$(BUILD_DIR)/kernel.o: $(SRC_DIR)/kernel/kernel.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/ports.o: $(SRC_DIR)/drivers/ports.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/screen.o: $(SRC_DIR)/drivers/screen.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/memory.o: $(SRC_DIR)/memory/memory.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/idt.o: $(SRC_DIR)/interrupt/idt.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/isr_c.o: $(SRC_DIR)/interrupt/isr.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/timer.o: $(SRC_DIR)/drivers/timer.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/keyboard.o: $(SRC_DIR)/drivers/keyboard.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/k_entry.o $(BUILD_DIR)/kernel.o $(BUILD_DIR)/ports.o $(BUILD_DIR)/screen.o $(BUILD_DIR)/memory.o $(BUILD_DIR)/idt.o $(BUILD_DIR)/isr.o $(BUILD_DIR)/isr_c.o $(BUILD_DIR)/timer.o $(BUILD_DIR)/keyboard.o
	$(LD) -m elf_i386 -o $(BUILD_DIR)/kernel.elf -T $(SRC_DIR)/kernel/linker.ld $^
	objcopy -O binary -j .text -j .rodata -j .eh_frame -j .data -j .bss $(BUILD_DIR)/kernel.elf $@

$(BUILD_DIR)/os-image.bin: $(BUILD_DIR)/boot.bin $(BUILD_DIR)/kernel.bin
	cat $^ > $@
	truncate -s 1440k $@

run: $(BUILD_DIR)/os-image.bin
	qemu-system-i386 -drive format=raw,file=$(BUILD_DIR)/os-image.bin,index=0,if=floppy -boot a

clean:
	rm -rf $(BUILD_DIR)
