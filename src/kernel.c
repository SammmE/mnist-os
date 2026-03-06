#include "fat32.h"
#include "math.h"
#include "types.h"
#include "vga.h"

float network_weights[7510];

float *W1;
float *b1;
float *W2;
float *b2;

void kernel_main() {
  clear_screen();
  println_string("Booting MNIST-OS");

  fpu_init();
  fat32_init();

  struct DirectoryEntry test_file;

  if (fat32_find_file("WEIGHTS BIN", &test_file)) {
    print_string("Found Weights! Size: ");
    println_hex(test_file.size);

    // W_1 = 64 * 100 = 6400 floats
    // b_1 =            100 floats
    // W_2 = 100 * 10 = 1000 floats
    // W_1 =            10 floats
    // TOTAL =          30040 bytes
    uint8_t file_data[30040];

    fat32_read_file(&test_file, (uint8_t *)network_weights);

    W1 = network_weights;
    b1 = W1 + (64 * 100);
    W2 = b1 + 100;
    b2 = W2 + (100 * 10);
  }

  float a = 6.7f;
  float b = 4.1f;
  float result = a * b;
  uint32_t int_result = (uint32_t)result;
  print_hex_byte(int_result);
}
