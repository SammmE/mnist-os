BUILD_DIR = build
SRC_DIR = src

AS = nasm
CC = gcc
LD = ld

CFLAGS = -m32 -ffreestanding -c -Os -nostdlib -fno-pie
LDFLAGS = -m elf_i386 -T $(SRC_DIR)/linker.ld -nostdlib

all: $(BUILD_DIR)/os-image.bin

run: all
	qemu-system-i386 -drive format=raw,file=$(BUILD_DIR)/os-image.bin,index=0,if=floppy -m 128M

$(BUILD_DIR)/os-image.bin: $(BUILD_DIR)/boot.bin $(BUILD_DIR)/kernel.bin
	cat $(BUILD_DIR)/boot.bin $(BUILD_DIR)/kernel.bin > $(BUILD_DIR)/os-image.bin
	# Ensure the binary is large enough for the floppy disk read (15 sectors)
	truncate -s 8192 $(BUILD_DIR)/os-image.bin

$(BUILD_DIR)/boot.bin: $(SRC_DIR)/boot.asm | $(BUILD_DIR)
	$(AS) -f bin $(SRC_DIR)/boot.asm -o $(BUILD_DIR)/boot.bin

$(BUILD_DIR)/kernel_entry.o: $(SRC_DIR)/kernel_entry.asm | $(BUILD_DIR)
	$(AS) -f elf32 $(SRC_DIR)/kernel_entry.asm -o $(BUILD_DIR)/kernel_entry.o

$(BUILD_DIR)/kernel.o: $(SRC_DIR)/kernel.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/kernel.c -o $(BUILD_DIR)/kernel.o

$(BUILD_DIR)/kernel.bin: $(BUILD_DIR)/kernel_entry.o $(BUILD_DIR)/kernel.o $(SRC_DIR)/linker.ld | $(BUILD_DIR)
	$(LD) $(LDFLAGS) -o $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel_entry.o $(BUILD_DIR)/kernel.o
	objcopy -O binary $(BUILD_DIR)/kernel.elf $(BUILD_DIR)/kernel.bin

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
