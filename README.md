# MNIST-OS

A bare-metal x86 32-bit operating system that performs handwritten digit classification using a neural network. The OS boots directly on hardware, loads a training dataset from a FAT32 filesystem, trains the model on-device, and then runs inference on a held-out test digit.

## Overview

MNIST-OS demonstrates neural network training and inference in a minimal operating system environment without relying on any existing OS kernel or standard libraries. The project includes a custom bootloader, FAT32 filesystem driver, VGA text-mode display, and a neural network implementation written from scratch in assembly and C.

## Requirements

- NASM assembler
- GCC with 32-bit support
- GNU binutils (`ld`, `objcopy`)
- `mtools` for FAT32 image manipulation
- QEMU for emulation
- Python 3 with `numpy` and `scikit-learn` for dataset export

On Debian/Ubuntu systems:
```bash
sudo apt install nasm gcc-multilib binutils mtools qemu-system-x86 python3-sklearn python3-numpy
```

## Building and Running

### Export the Dataset

Python prepares the training and test data, normalizes it, and writes FAT32-friendly binary blobs into `disk/`:

```bash
make dataset
```

You can tune the exported configuration directly:

```bash
python3 model/bins.py --output-dir disk --hidden-size 32 --epochs 8 --learning-rate 0.015 --sample-index 4
```

The exporter writes:
- `CONFIG.BIN` with model dimensions, dataset counts, epoch count, selected sample, and learning rate
- `TRAIN.BIN` with float32 training features
- `TRNLABEL.BIN` with uint8 training labels
- `TEST.BIN` with float32 test features
- `TSTLABEL.BIN` with uint8 test labels

### Build the OS

```bash
make
```

This creates a 96MB disk image at `build/os.img`.

### Run in QEMU

```bash
make run
```

The system boots, loads the dataset from the FAT32 partition, initializes the model, trains it in the kernel, and then displays:
- training hyperparameters and elapsed training time
- per-epoch confidence and train/test accuracy
- the selected test image
- the final prediction and inference timing

## Changing the Training Run

Regenerate the dataset binaries with different hyperparameters or a different held-out image, then rebuild and run:

```bash
python3 model/bins.py --output-dir disk --sample-index 12 --hidden-size 48 --epochs 10 --learning-rate 0.01
make run
```

## Architecture

### Boot Process

1. BIOS loads the first 512 bytes (bootloader) from `src/boot.asm` to memory address `0x7C00`
2. The bootloader loads the kernel from disk and switches into 32-bit protected mode
3. The kernel initializes the FPU, timer, and FAT32 filesystem
4. The kernel loads the dataset/config files, trains the network, and performs inference on a held-out sample

### Filesystem

The OS includes a minimal FAT32 driver in `src/fat32.c` that can:
- Parse the MBR to locate the FAT32 partition
- Read the BIOS Parameter Block
- Traverse directory entries in the root directory
- Read files by following cluster chains

The FAT32 partition begins at sector 2048 and contains the exported dataset and config binaries.

### Neural Network

The kernel-side neural network engine in `src/nn.c` is configured at runtime:

- Input layer: 64 features (8x8 grayscale pixels)
- Hidden layer: configurable, exported from Python
- Output layer: 10 neurons with softmax activation

Network architecture:

```text
Input(64) -> FC -> ReLU(H) -> FC + Softmax(10)
```

The kernel initializes weights randomly, trains with stochastic gradient descent, and reports per-epoch statistics on the VGA console.

### Dataset File Format

`CONFIG.BIN` stores:
- magic/version for validation
- input, hidden, and output sizes
- train/test sample counts
- epoch count
- displayed test-sample index
- learning rate

`TRAIN.BIN` stores `train_count * input_size` float32 values.

`TRNLABEL.BIN` stores `train_count` uint8 labels.

`TEST.BIN` stores `test_count * input_size` float32 values.

`TSTLABEL.BIN` stores `test_count` uint8 labels.

All floating-point values are stored as 32-bit little-endian IEEE 754 values.

### Display

The VGA driver in `src/vga.c` uses text mode to:
- Display boot and filesystem information
- Show per-epoch training statistics
- Render the selected test image as ASCII characters
- Show the final prediction and timing

## Technical Details

- Architecture: x86 (32-bit protected mode)
- Bootloader: single-stage, written in NASM assembly
- Kernel: freestanding C with a custom entry point and no standard library
- Compiler flags: `-ffreestanding -nostdlib -fno-pie -fno-stack-protector`
- Disk layout: MBR at sector 0, kernel at sector 1, FAT32 partition at sector 2048
- Disk size: 96MB master image with a 95MB FAT32 partition
- Accuracy: varies by training run and exported hyperparameters

## License

You can do anything you want!
