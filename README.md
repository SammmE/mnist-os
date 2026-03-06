
# MNIST-OS

A bare-metal x86 32-bit operating system that performs handwritten digit classification using a neural network. The OS boots directly on hardware, loads a pre-trained model from a FAT32 filesystem, and runs inference to classify MNIST digits.

## Overview

MNIST-OS demonstrates neural network inference in a minimal operating system environment without relying on any existing OS kernel or standard libraries. Because I decided not to use any external libraries, the system includes a custom bootloader, FAT32 filesystem driver, VGA text-mode display, and a neural network implementation. All were written from scratch in assembly and C.

## Requirements

- NASM assembler
- GCC (with 32-bit support)
- GNU binutils (ld, objcopy)
- mtools (for FAT32 image manipulation)
- QEMU (for emulation)
- Python 3 with scikit-learn and numpy (for training)

On Debian/Ubuntu systems:
```bash
sudo apt install nasm gcc-multilib binutils mtools qemu-system-x86 python3-sklearn python3-numpy
```

## Building and Running

### Generate Model Weights

The neural network must be trained before building the OS image:

```bash
cd model
python3 bins.py
mv WEIGHTS.BIN ../disk
mv IMAGE.BIN ../disk
```

This script:
- Trains a 2-layer neural network on the MNIST digits dataset
- Exports the model weights and biases to `WEIGHTS.BIN`
- Exports a test image to `IMAGE.BIN`
- Both files are placed in the `disk/` directory

### Build the OS

```bash
make
```

This creates a 65MB disk image at `build/os.img`

### Run in QEMU

```bash
make run
```

The system will boot, load the neural network weights and test image from the FAT32 partition, perform inference, and display the input digit along with classification probabilities.

## Changing the Test Image

To classify a different digit, modify the `IMAGE` constant in `model/bins.py`:

```python
IMAGE = 0  # Change to any index in the test set
```

Then regenerate the binaries and rebuild:

```bash
cd model
python3 bins.py
mv WEIGHTS.BIN ../disk
mv IMAGE.BIN ../disk
cd ..
make clean
make run
```

## Architecture

### Boot Process

1. **BIOS** loads the first 512 bytes (bootloader) from `src/boot.asm` to memory address 0x7C00
2. **Bootloader** loads 45 sectors containing the kernel starting at sector 2
3. **Bootloader** enables the A20 line, sets up a Global Descriptor Table, and switches to 32-bit protected mode
4. **Kernel** initializes the FPU, parses the FAT32 partition, loads the model and image files, and performs inference

### Filesystem

The OS includes a minimal FAT32 driver (`src/fat32.c`) that can:
- Parse the MBR to locate the FAT32 partition
- Read the BIOS Parameter Block
- Traverse directory entries in the root directory
- Read files by following cluster chains
- NOTE: This fat.c is the most basic form, just enough utility to read a load known files, do not use it in your OS

The FAT32 partition begins at sector 2048 and contains the neural network weights and test image as binary files.

### Neural Network

The inference engine (`src/nn.c`) implements a 2-layer feedforward network:

- **Input layer**: 64 features (8×8 grayscale pixel values)
- **Hidden layer**: 100 neurons with ReLU activation
- **Output layer**: 10 neurons (digits 0-9) with softmax activation

Network architecture:
```
Input(64) -> FC -> ReLU(100) -> FC + Softmax(10)
```

Mathematical operations (`src/math.c`) are implemented using x87 FPU instructions for floating-point matrix multiplication, bias addition, ReLU, and softmax. They're as efficient as I can get them to be.

### Model File Format

**WEIGHTS.BIN** (30,040 bytes):
- W1: 6,400 floats (64×100 weight matrix)
- b1: 100 floats (bias vector)
- W2: 1,000 floats (100×10 weight matrix)
- b2: 10 floats (bias vector)

**IMAGE.BIN** (256 bytes):
- 64 floats representing 8×8 pixel values

All values are stored as 32-bit little-endian IEEE 754 floating-point numbers.

### Display

The VGA driver (`src/vga.c`) uses text mode to:
- Display boot messages and filesystem information
- Render the input image as ASCII characters
- Show classification probabilities as horizontal bar charts

## Technical Details

- **Architecture**: x86 (32-bit protected mode)
- **Bootloader**: Single-stage, written in NASM assembly
- **Kernel**: Freestanding C with custom entry point, no standard library
- **Compiler flags**: `-ffreestanding -nostdlib -fno-pie -fno-stack-protector`
- **Disk layout**: MBR at sector 0, kernel at sector 1, FAT32 partition at sector 2048
- **Test accuracy**: ~95% on MNIST test set (varies by training run)

## License

You can do anything you want!
