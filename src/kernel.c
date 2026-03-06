#include "fat32.h"
#include "math.h"
#include "nn.h"
#include "vga.h"

float weights[7510];
float image[64];
float output_layer[10];

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

    fat32_read_file(&test_file, (uint8_t *)weights);

    W1 = weights;
    b1 = W1 + (64 * 100);
    W2 = b1 + 100;
    b2 = W2 + (100 * 10);
  }

  struct DirectoryEntry image_file;
  if (fat32_find_file("IMAGE   BIN", &image_file)) {
    print_string("Found image! Size: ");
    println_hex(image_file.size); // Should print 0x100 (which is 256 in hex)

    // Read the binary data directly into our input array
    fat32_read_file(&image_file, (uint8_t *)image);
    println_string("Image loaded successfully!");
  }

  clear_screen();

  println_image(image);

  infer_number(image, W1, b1, W2, b2, output_layer);

  for (int i = 0; i < 10; i++) {
    uint32_t percent = (uint32_t)(output_layer[i] * 100.0f);
    if (percent) {
      print_char(i + '0');
      print_string(": ");

      for (int i = 0; i < percent / 10; i++)
        print_char('*');
      println_string("");
    }
  }
}
