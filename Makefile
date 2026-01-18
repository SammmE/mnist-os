# Ensure these point to your actual cross-compiler binaries
CC = i686-elf-gcc
LD = i686-elf-ld
ASM = nasm

SRC_DIR = src
BUILD_DIR = build

CFLAGS = -ffreestanding -m32 -g -c -I$(SRC_DIR)

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

$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/k_entry.o $(BUILD_DIR)/kernel.o
	$(LD) -o $@ -Ttext 0x1000 $^ --oformat binary

$(BUILD_DIR)/os-image.bin: $(BUILD_DIR)/boot.bin $(BUILD_DIR)/kernel.bin
	cat $^ > $@

run: $(BUILD_DIR)/os-image.bin
	qemu-system-i386 -fda $(BUILD_DIR)/os-image.bin

clean:
	rm -rf $(BUILD_DIR)
