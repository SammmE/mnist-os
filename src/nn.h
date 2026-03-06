#ifndef NN_H
#define NN_H

#include "types.h"

void infer_number(float *image, float *W1, float *b1, float *W2, float *b2,
                  float *output_layer);

#endif // !NN_H
