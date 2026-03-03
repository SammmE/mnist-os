# MNIST-OS: Bare Metal AI Operating System Roadmap

This is an incredibly ambitious, old-school hacker project. Stripping away the modern comforts of standard libraries, GRUB, and user space to build an AI-capable OS directly on the bare metal is a fantastic way to master systems programming and computer architecture. 

Because we are skipping user space and paging, everything will run in Ring 0 (kernel mode) with a flat memory model, which simplifies things immensely.

---

## Phase 1: Bootstrapping and Bare Metal Output

Your first goal is to get out of 16-bit real mode, into 32-bit protected mode, and execute C code that can print to the screen. 



* **Stage 1 Bootloader (16-bit Assembly):** You have 512 bytes in the MBR. Write a simple BIOS interrupt-driven loader (`int 0x13`) that reads a few contiguous sectors from the disk into memory and jumps to them. 
* **Stage 2 Bootloader (16-bit to 32-bit Assembly):** * Enable the A20 line.
  * Load a basic flat Global Descriptor Table (GDT) defining a massive 4GB code segment and 4GB data segment.
  * Set the first bit of the `cr0` control register to switch to 32-bit Protected Mode.
  * Set up a basic stack and `jmp` to your C kernel entry point.
* **Freestanding C Environment:** Compile your C code with `-ffreestanding`, `-nostdlib`, and `-m32`. You will also need a custom linker script (`linker.ld`) to ensure your kernel entry point is placed exactly where the bootloader jumps.
* **VGA Text Mode Driver:** Since you have no `printf`, write a simple function that writes characters directly to VGA memory at `0xB8000`.

---

## Phase 2: Disk I/O (ATA PIO)

To read your FAT32 file system, you first need to be able to talk to the hard drive directly without BIOS interrupts.

* **I/O Port Functions:** Implement basic inline assembly functions for `inb` (read byte from port) and `outb` (write byte to port).
* **ATA PIO Mode Driver:** Write a driver that communicates with the IDE controller via I/O ports (e.g., ports `0x1F0` through `0x1F7`).
* **Sector Reading:** Create a function `read_sector(uint32_t LBA, uint8_t *buffer)` that reads 512-byte blocks from the QEMU hard disk image into a memory buffer.

---

## Phase 3: The FAT32 File System Driver

Now that you can read raw sectors, you need to understand the data structures on the disk to find your files.



* **MBR Parsing:** Read Sector 0, look at the partition table at offset `0x1BE`, and find the LBA (Logical Block Address) of the first FAT32 partition.
* **Volume ID (Boot Sector):** Read the first sector of the FAT32 partition. Parse the BIOS Parameter Block (BPB) to find the sector size, sectors per cluster, the location of the File Allocation Tables (FAT), and the location of the Root Directory.
* **Path Resolution & Cluster Chaining:** Write a function to search directory entries (32 bytes each) for your target file names (e.g., `WEIGHTS.BIN`, `IMAGE.BIN`). Once found, read the file by following its cluster chain in the FAT until the End of Cluster (EOC) mark.

---

## Phase 4: Math and CPU Setup (No `libm`)

To run a neural network, you need floats and math functions.

* **FPU Initialization:** Write a small assembly stub using the `finit` instruction and clearing the EM (Emulation) bit in the `cr0` register. This allows GCC to use hardware floating-point instructions natively in your C code.
* **Custom Math Functions:** For your activation functions, you will need to roll your own math. 
  * **ReLU:** Trivial. $y = \max(0, x)$.
  * **Sigmoid / Softmax:** You will need an exponential function. Since you don't have `exp()` from `<math.h>`, write a simple Taylor series expansion for $e^x$, or use a pre-computed lookup table loaded into memory.

---

## Phase 5: Neural Network Implementation

Keep the architecture extremely simple. A multi-layer perceptron (MLP) with one hidden layer (e.g., 784 input nodes -> 128 hidden nodes -> 10 output nodes) is perfect.

* **Host-Side Prep:** Train your network on your host machine using Python/PyTorch. Export the trained weights and biases as a raw, flat binary file (e.g., a sequence of 32-bit floats). Do the same for a few test images (flattened 28x28 = 784 bytes).
* **Kernel Loading:** Use your FAT32 driver to load `WEIGHTS.BIN` into a pre-allocated array in memory. Load your target `IMAGE.BIN` into an input array.
* **Inference Loop:** Write standard C loops to perform the matrix multiplications and additions:
  * $Z_1 = X \cdot W_1 + B_1$
  * $A_1 = \text{ReLU}(Z_1)$
  * $Z_2 = A_1 \cdot W_2 + B_2$
* **Output:** Find the index (0-9) of the maximum value in $Z_2$ and print it to your VGA text screen using your custom print function.

---

## The QEMU Testing Loop

To iterate quickly, use a `Makefile` that compiles your code, concatenates your bootloader and kernel, generates a `.img` file, formats it as FAT32, copies your weights/images into it, and runs:

`qemu-system-i386 -drive format=raw,file=os.img`
