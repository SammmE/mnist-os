AS = nasm
CC = gcc
LD = ld
OBJCOPY = objcopy

CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -nostdlib -mno-sse -mno-mmx
LDFLAGS = -m elf_i386 -T src/linker.ld

SRC_DIR = src
BUILD_DIR = build
DISK_SRC = disk

# disk image
OS_IMAGE = $(BUILD_DIR)/os.img

BOOT_BIN = $(BUILD_DIR)/boot.bin
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
KERNEL_ELF = $(BUILD_DIR)/kernel.elf

# Object files for kernel
C_SOURCES = $(wildcard $(SRC_DIR)/*.c)
C_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(C_SOURCES))
OBJS = $(BUILD_DIR)/kernel_entry.o $(C_OBJS)

all: $(OS_IMAGE)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BOOT_BIN): $(SRC_DIR)/boot.asm | $(BUILD_DIR)
	$(AS) -f bin $< -o $@

$(BUILD_DIR)/kernel_entry.o: $(SRC_DIR)/kernel_entry.asm | $(BUILD_DIR)
	$(AS) -f elf32 $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL_ELF): $(OBJS) src/linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@
$(OS_IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	@echo "1. Creating solid 65MB Master disk image..."
	dd if=/dev/zero of=$@ bs=1M count=65

	@echo "2. Creating and Formatting FAT32 partition..."
	dd if=/dev/zero of=$(BUILD_DIR)/fat32.img bs=1M count=64
	mkfs.fat -F 32 -I $(BUILD_DIR)/fat32.img

	@echo "3. Copying files into FAT32 image..."
	mkdir -p $(DISK_SRC)
	mcopy -i $(BUILD_DIR)/fat32.img -s $(DISK_SRC)/* ::/

	@echo "4. Injecting pieces into the Master Disk..."
	# Write MBR at Sector 0
	dd if=$(BOOT_BIN) of=$@ bs=512 seek=0 conv=notrunc
	# Write Kernel at Sector 1
	dd if=$(KERNEL_BIN) of=$@ bs=512 seek=1 conv=notrunc
	# Paste the FAT32 volume at exactly Sector 2048
	dd if=$(BUILD_DIR)/fat32.img of=$@ bs=512 seek=2048 conv=notrunc

run: all
	qemu-system-i386 -drive format=raw,file=$(OS_IMAGE)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run clean
