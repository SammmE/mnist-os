#ifndef MATH_H
#define MATH_H

void fpu_init();

// basics
float math_absf(float x);          // abs value of a float
float math_maxf(float a, float b); // max of 2 floats
float math_expf(float x);          // e^x (will use taylor series for ts)

// activation functions
float math_relu(float x);
void math_relu_layer(float *layer,
                     int size); // relu on the entire array, in-place
float math_sigmoid(float x);    // 1 / (1 + e^-x)
void math_sigmoid_layer(float *layer, int size); // sigmoid an entire array, in place
void math_softmax(float *logits, int size);

// lin alg
void math_matmul(const float *A, const float *B, float *C, int rows_A,
                 int cols_A, int cols_B); // C = A * B
void math_add_bias(float *target, const float *bias,
                   int size); // add bias vector

// helpers
int math_argmax(const float *array, int size); // return highest index
float math_max_array(const float *array, int size); // return highest value
void math_clear_array(float *array, int size); // clear an array

#endif // !MATH_H
