C_SOURCES = $(wildcard src/*.c)
HEADERS = $(wildcard src/*.h)
BUILD_DIR = build
OBJ = $(patsubst src/%.c, $(BUILD_DIR)/%.o, $(C_SOURCES))

CC = gcc
GDB = gdb
CFLAGS = -m32 -fno-pie -ffreestanding -c -g
LDFLAGS = -m elf_i386 -T src/link.ld

# Default make target
all: $(BUILD_DIR)/os-image.bin

# Run in QEMU
run: all
	qemu-system-x86_64 -drive format=raw,file=$(BUILD_DIR)/os-image.bin

# Build the os image
$(BUILD_DIR)/os-image.bin: $(BUILD_DIR)/boot.bin $(BUILD_DIR)/kernel.bin
	dd if=/dev/zero of=$@ bs=512 count=2048
	dd if=$(BUILD_DIR)/boot.bin of=$@ conv=notrunc
	dd if=$(BUILD_DIR)/kernel.bin of=$@ seek=1 conv=notrunc

# Compile C kernel
$(BUILD_DIR)/kernel.bin: $(OBJ)
	ld -o $@ $^ $(LDFLAGS)

# Compile C source files
$(BUILD_DIR)/%.o: src/%.c ${HEADERS}
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Assemble bootloader
$(BUILD_DIR)/boot.bin: src/boot.asm
	mkdir -p $(BUILD_DIR)
	nasm $< -f bin -o $@

# Clean
clean:
	rm -rf $(BUILD_DIR)
