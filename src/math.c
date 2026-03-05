#include "math.h"
#include "types.h"

void fpu_init() {
  uint32_t cr0;

  __asm__ __volatile__("mov %%cr0, %0" : "=r"(cr0));
  cr0 &= ~(1 << 2);
  cr0 |= (1 << 1);
  __asm__ __volatile__("mov %0, %%cr0" ::"r"(cr0));
  __asm__ __volatile__("fninit");
}

// basics
float math_absf(float x) {
  if (x < 0)
    return -1 * x;

  return x;
}

float math_maxf(float a, float b) {
  if (a > b)
    return a;
  return b;
}
float math_expf(float x); // e^x (will use taylor series for ts)

// activation functions
float math_relu(float x) { return math_maxf(0.0f, x); }

void math_relu_layer(float *layer, int size) {
  for (int i = 0; i < size; i++) {
    layer[i] = math_relu(layer[i]);
  }
}

float math_sigmoid(float x) { return 1 / (1 + (1 / math_expf(x))); }
void math_softmax(float *logits, int size);

// lin alg
void math_matmul(const float *A, const float *B, float *C, int rows_A,
                 int cols_A, int cols_B); // C = A * B
void math_add_bias(float *target, const float *bias,
                   int size); // add bias vector

// helpers
int math_argmax(const float *array, int size) {
  float val = array[0];
  int idx = 0;
  for (int i = 0; i < size; i++) {
    if (array[i] > val) {
      idx = i;
      val = array[i];
    }
  }

  return idx;
}
