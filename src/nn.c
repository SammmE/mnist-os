#include "nn.h"
#include "math.h"
#include "types.h"

static uint32_t nn_next_random(uint32_t *seed) {
  *seed = (*seed * 1664525U) + 1013904223U;
  return *seed;
}

void nn_init(struct NeuralNet *net, const struct ModelConfig *config, float *W1,
             float *b1, float *W2, float *b2) {
  net->input_size = config->input_size;
  net->hidden_size = config->hidden_size;
  net->output_size = config->output_size;
  net->W1 = W1;
  net->b1 = b1;
  net->W2 = W2;
  net->b2 = b2;
}

void nn_randomize(struct NeuralNet *net, uint32_t *seed) {
  float hidden_scale = 0.15f;
  float output_scale = 0.20f;

  for (uint32_t i = 0; i < net->input_size * net->hidden_size; i++) {
    float unit =
        ((nn_next_random(seed) >> 8) & 0x00FFFFFFU) * (1.0f / 16777216.0f);
    net->W1[i] = ((unit * 2.0f) - 1.0f) * hidden_scale;
  }

  for (uint32_t i = 0; i < net->hidden_size; i++) {
    net->b1[i] = 0.0f;
  }

  for (uint32_t i = 0; i < net->hidden_size * net->output_size; i++) {
    float unit =
        ((nn_next_random(seed) >> 8) & 0x00FFFFFFU) * (1.0f / 16777216.0f);
    net->W2[i] = ((unit * 2.0f) - 1.0f) * output_scale;
  }

  for (uint32_t i = 0; i < net->output_size; i++) {
    net->b2[i] = 0.0f;
  }
}

void nn_predict(const struct NeuralNet *net, const float *input, float *hidden,
                float *output) {
  math_clear_array(hidden, (int)net->hidden_size);
  math_clear_array(output, (int)net->output_size);

  math_matmul(input, net->W1, hidden, 1, (int)net->input_size,
              (int)net->hidden_size);
  math_add_bias(hidden, net->b1, (int)net->hidden_size);
  math_relu_layer(hidden, (int)net->hidden_size);

  math_matmul(hidden, net->W2, output, 1, (int)net->hidden_size,
              (int)net->output_size);
  math_add_bias(output, net->b2, (int)net->output_size);
  math_softmax(output, (int)net->output_size);
}

float nn_train_epoch(struct NeuralNet *net, const float *inputs,
                     const uint8_t *labels, const uint32_t *sample_order,
                     uint32_t sample_count, float learning_rate, float *hidden,
                     float *output, float *output_grad, float *hidden_grad) {
  float avg_true_confidence = 0.0f;

  for (uint32_t sample_idx = 0; sample_idx < sample_count; sample_idx++) {
    uint32_t row = sample_order ? sample_order[sample_idx] : sample_idx;
    const float *input = inputs + (row * net->input_size);
    uint8_t label = labels[row];

    nn_predict(net, input, hidden, output);
    avg_true_confidence += output[label];

    for (uint32_t j = 0; j < net->output_size; j++) {
      output_grad[j] = output[j];
    }
    output_grad[label] -= 1.0f;

    for (uint32_t i = 0; i < net->hidden_size; i++) {
      float grad = 0.0f;
      float relu_grad = hidden[i] > 0.0f ? 1.0f : 0.0f;

      for (uint32_t j = 0; j < net->output_size; j++) {
        grad += output_grad[j] * net->W2[i * net->output_size + j];
      }
      hidden_grad[i] = grad * relu_grad;
    }

    for (uint32_t i = 0; i < net->hidden_size; i++) {
      for (uint32_t j = 0; j < net->output_size; j++) {
        net->W2[i * net->output_size + j] -=
            learning_rate * hidden[i] * output_grad[j];
      }
    }

    for (uint32_t j = 0; j < net->output_size; j++) {
      net->b2[j] -= learning_rate * output_grad[j];
    }

    for (uint32_t i = 0; i < net->input_size; i++) {
      for (uint32_t j = 0; j < net->hidden_size; j++) {
        net->W1[i * net->hidden_size + j] -=
            learning_rate * input[i] * hidden_grad[j];
      }
    }

    for (uint32_t j = 0; j < net->hidden_size; j++) {
      net->b1[j] -= learning_rate * hidden_grad[j];
    }
  }

  if (sample_count == 0) {
    return 0.0f;
  }

  return avg_true_confidence / (float)sample_count;
}

uint32_t nn_count_correct(const struct NeuralNet *net, const float *inputs,
                          const uint8_t *labels, uint32_t sample_count,
                          float *hidden, float *output) {
  uint32_t correct = 0;

  for (uint32_t i = 0; i < sample_count; i++) {
    nn_predict(net, inputs + (i * net->input_size), hidden, output);
    if ((uint8_t)math_argmax(output, (int)net->output_size) == labels[i]) {
      correct++;
    }
  }

  return correct;
}
