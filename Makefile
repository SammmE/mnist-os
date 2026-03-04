AS = nasm
CC = gcc
LD = ld
OBJCOPY = objcopy

CFLAGS = -m32 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -nostdlib
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
	@echo "Creating 64MB empty disk image..."
	dd if=/dev/zero of=$@ bs=1M count=64
	
	@echo "Formatting with FAT32..."

	# -F 32 = FAT32, -i = Volume ID, -I = Ignore partitions
	mkfs.fat -F 32 -I $@
	
	@echo "Copying files from ./$(DISK_SRC) into the image..."

	# mcopy -i [image] -s [source] ::[destination]
	mcopy -i $@ -s $(DISK_SRC)/* ::/

	@echo "Injecting Bootloader and Kernel into the start of the disk..."

	# conv=notrunc - don't delete the rest of the 64MB file
	dd if=$(BOOT_BIN) of=$@ conv=notrunc

	# Start writing kernel at Sector 1 (after MBR)
	dd if=$(KERNEL_BIN) of=$@ seek=1 conv=notrunc

run: all
	qemu-system-i386 -drive format=raw,file=$(OS_IMAGE)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run clean
