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
  union {
    float f;
    uint32_t i;
  } u;

  u.f = x;
  u.i &= 0x7FFFFFFF;

  return u.f;
}

float math_maxf(float a, float b) { return (a > b) ? a : b; }

float math_expf(float x) {
  // Schraudolph Algorithm
  union {
    float f;
    unsigned int i;
  } u;

  u.i = (unsigned int)(12102203.0f * x + 1064866805.0f);

  return u.f;
}

// activation functions
float math_relu(float x) { return math_maxf(0.0f, x); }

void math_relu_layer(float *layer, int size) {
  for (int i = 0; i < size; i++) {
    layer[i] = math_relu(layer[i]);
  }
}

float math_sigmoid(float x) { return 1.0f / (1.0f + math_expf(-x)); }

void math_sigmoid_layer(float *layer, int size) {
  for (int i = 0; i < size; i++) {
    layer[i] = math_sigmoid(layer[i]);
  }
}

void math_softmax(float *logits, int size) {
  float max_val = logits[0];
  for (int i = 1; i < size; i++) {
    if (logits[i] > max_val) {
      max_val = logits[i];
    }
  }

  // subtract max to stop overflow
  float sum = 0.0f;
  for (int i = 0; i < size; i++) {
    logits[i] = math_expf(logits[i] - max_val);
    sum += logits[i];
  }

  // add 1e-7f to prevend div by 0
  float inv_sum = 1.0f / (sum + 1e-7f);
  for (int i = 0; i < size; i++) {
    logits[i] *= inv_sum;
  }
}

// lin alg
void math_matmul(const float *A, const float *B, float *C, int rows_A,
                 int cols_A, int cols_B) {
  for (int i = 0; i < rows_A; ++i) {
    for (int k = 0; k < cols_A; ++k) {
      float a_ik = A[i * cols_A + k];
      for (int j = 0; j < cols_B; ++j) {
        C[i * cols_B + j] += a_ik * B[k * cols_B + j];
      }
    }
  }
}

void math_add_bias(float *target, const float *bias, int size) {
  for (int i = 0; i < size; i++)
    target[i] += bias[i];
}

// helpers
int math_argmax(const float *array, int size) {
  float val = array[0];
  int idx = 0;

  for (int i = 1; i < size; i++) {
    if (array[i] > val) {
      idx = i;
      val = array[i];
    }
  }
  return idx;
}

float math_max_array(const float *array, int size) {
  float val = array[0];
  int idx = 0;

  for (int i = 1; i < size; i++) {
    if (array[i] > val) {
      idx = i;
      val = array[i];
    }
  }
  return val;
}

void math_clear_array(float *array, int size) {
  for (int i = 0; i < size; i++)
    array[i] = 0;
}
