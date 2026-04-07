#ifndef NN_H
#define NN_H

#include "types.h"

#define NN_MAX_INPUT_SIZE 64
#define NN_MAX_HIDDEN_SIZE 64
#define NN_MAX_OUTPUT_SIZE 10
#define NN_MAX_TRAIN_SAMPLES 1500
#define NN_MAX_TEST_SAMPLES 400
#define NN_MAX_EPOCHS 12
#define NN_CONFIG_MAGIC 0x534F4E4DU
#define NN_CONFIG_VERSION 1U

struct ModelConfig {
  uint32_t magic;
  uint32_t version;
  uint32_t input_size;
  uint32_t hidden_size;
  uint32_t output_size;
  uint32_t train_count;
  uint32_t test_count;
  uint32_t epochs;
  uint32_t sample_index;
  float learning_rate;
} __attribute__((packed));

struct NeuralNet {
  uint32_t input_size;
  uint32_t hidden_size;
  uint32_t output_size;
  float *W1;
  float *b1;
  float *W2;
  float *b2;
};

void nn_init(struct NeuralNet *net, const struct ModelConfig *config, float *W1,
             float *b1, float *W2, float *b2);
void nn_randomize(struct NeuralNet *net, uint32_t *seed);
void nn_predict(const struct NeuralNet *net, const float *input, float *hidden,
                float *output);
float nn_train_epoch(struct NeuralNet *net, const float *inputs,
                     const uint8_t *labels, const uint32_t *sample_order,
                     uint32_t sample_count, float learning_rate, float *hidden,
                     float *output, float *output_grad, float *hidden_grad);
uint32_t nn_count_correct(const struct NeuralNet *net, const float *inputs,
                          const uint8_t *labels, uint32_t sample_count,
                          float *hidden, float *output);

#endif // !NN_H
