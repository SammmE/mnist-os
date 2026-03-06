#include "nn.h"
#include "math.h"
#include "types.h"

void infer_number(float *image, float *W1, float *b1, float *W2, float *b2,
                  float *output_layer) {
  float hidden_layer[100];
  math_clear_array(hidden_layer, 100);
  math_clear_array(output_layer, 10);

  // HIDDEN LAYER - Z1  = X * W1 + B

  // Z1 = X * W1
  math_matmul(image, W1, hidden_layer, 1, 64, 100);

  // Z1 = Z1 + b1
  math_add_bias(hidden_layer, b1, 100);

  // A1 = ReLU(Z1)
  math_relu_layer(hidden_layer, 100);

  // OUTPUT LAYER - Z2 = A1 * W2 + B2

  // Z2 = A1 * W2
  math_matmul(hidden_layer, W2, output_layer, 1, 100, 10);

  // Z2 = Z2 + b1
  math_add_bias(output_layer, b2, 10);

  // softmax
  math_softmax(output_layer, 10);
}
