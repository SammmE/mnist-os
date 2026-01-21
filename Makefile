# Ensure these point to your actual cross-compiler binaries
CC = gcc
LD = ld
ASM = nasm

SRC_DIR = src
BUILD_DIR = build

CFLAGS = -ffreestanding -m32 -g -c -I$(SRC_DIR) -fno-pie -fno-pic

all: $(BUILD_DIR)/os-image.bin

$(BUILD_DIR)/boot.bin: $(SRC_DIR)/boot.asm
	@mkdir -p $(BUILD_DIR)
	$(ASM) -f bin $< -o $@

$(BUILD_DIR)/k_entry.o: $(SRC_DIR)/k_entry.asm
	@mkdir -p $(BUILD_DIR)
	$(ASM) -f elf $< -o $@

$(BUILD_DIR)/kernel.o: $(SRC_DIR)/kernel.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/ports.o: $(SRC_DIR)/drivers/ports.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/screen.o: $(SRC_DIR)/drivers/screen.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/k_entry.o $(BUILD_DIR)/kernel.o $(BUILD_DIR)/ports.o $(BUILD_DIR)/screen.o
	$(LD) -m elf_i386 -o $(BUILD_DIR)/kernel.elf -Ttext 0x1000 $^
	objcopy -O binary -j .text -j .rodata -j .data $(BUILD_DIR)/kernel.elf $@

$(BUILD_DIR)/os-image.bin: $(BUILD_DIR)/boot.bin $(BUILD_DIR)/kernel.bin
	cat $^ > $@
	truncate -s 1440k $@

run: $(BUILD_DIR)/os-image.bin
	qemu-system-i386 -drive format=raw,file=$(BUILD_DIR)/os-image.bin,index=0,if=floppy -boot a

clean:
	rm -rf $(BUILD_DIR)
