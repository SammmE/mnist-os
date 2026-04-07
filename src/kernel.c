#include "fat32.h"
#include "math.h"
#include "nn.h"
#include "timer.h"
#include "uint64.h"
#include "vga.h"

static struct ModelConfig config;
static struct NeuralNet net;

static float train_inputs[NN_MAX_TRAIN_SAMPLES * NN_MAX_INPUT_SIZE];
static uint8_t train_labels[NN_MAX_TRAIN_SAMPLES];
static float test_inputs[NN_MAX_TEST_SAMPLES * NN_MAX_INPUT_SIZE];
static uint8_t test_labels[NN_MAX_TEST_SAMPLES];

static float W1[NN_MAX_INPUT_SIZE * NN_MAX_HIDDEN_SIZE];
static float b1[NN_MAX_HIDDEN_SIZE];
static float W2[NN_MAX_HIDDEN_SIZE * NN_MAX_OUTPUT_SIZE];
static float b2[NN_MAX_OUTPUT_SIZE];

static float hidden_buffer[NN_MAX_HIDDEN_SIZE];
static float output_buffer[NN_MAX_OUTPUT_SIZE];
static float output_grad[NN_MAX_OUTPUT_SIZE];
static float hidden_grad[NN_MAX_HIDDEN_SIZE];
static uint32_t train_order[NN_MAX_TRAIN_SAMPLES];

static uint32_t train_accuracy_history[NN_MAX_EPOCHS];
static uint32_t test_accuracy_history[NN_MAX_EPOCHS];
static uint32_t confidence_history[NN_MAX_EPOCHS];

static int load_file_exact(const char *name, void *buffer, uint32_t expected_size) {
  struct DirectoryEntry file;

  if (!fat32_find_file(name, &file)) {
    print_string("Missing file: ");
    println_string(name);
    return 0;
  }

  if (file.size != expected_size) {
    print_string("Bad size for: ");
    println_string(name);
    return 0;
  }

  fat32_read_file(&file, (uint8_t *)buffer);
  return 1;
}

static void shuffle_indices(uint32_t *indices, uint32_t count, uint32_t *seed) {
  if (count < 2) {
    return;
  }

  for (uint32_t i = count - 1; i > 0; i--) {
    *seed = (*seed * 1664525U) + 1013904223U;
    uint32_t j = *seed % (i + 1);
    uint32_t tmp = indices[i];
    indices[i] = indices[j];
    indices[j] = tmp;
  }
}

static void print_scaled_thousandths(uint32_t scaled) {
  uint32_t whole = scaled / 1000U;
  uint32_t fraction = scaled % 1000U;

  print_uint32(whole);
  print_char('.');
  print_char('0' + (fraction / 100U));
  print_char('0' + ((fraction / 10U) % 10U));
  print_char('0' + (fraction % 10U));
}

static int validate_config(const struct ModelConfig *loaded) {
  if (loaded->magic != NN_CONFIG_MAGIC || loaded->version != NN_CONFIG_VERSION) {
    println_string("Unsupported dataset config");
    return 0;
  }
  if (loaded->input_size == 0 || loaded->hidden_size == 0 ||
      loaded->output_size == 0) {
    println_string("Invalid model dimensions");
    return 0;
  }
  if (loaded->input_size > NN_MAX_INPUT_SIZE ||
      loaded->hidden_size > NN_MAX_HIDDEN_SIZE ||
      loaded->output_size > NN_MAX_OUTPUT_SIZE) {
    println_string("Model exceeds kernel limits");
    return 0;
  }
  if (loaded->train_count == 0 || loaded->train_count > NN_MAX_TRAIN_SAMPLES ||
      loaded->test_count == 0 || loaded->test_count > NN_MAX_TEST_SAMPLES) {
    println_string("Dataset exceeds kernel limits");
    return 0;
  }
  if (loaded->epochs == 0 || loaded->epochs > NN_MAX_EPOCHS) {
    println_string("Epoch count exceeds display limits");
    return 0;
  }
  if (loaded->sample_index >= loaded->test_count) {
    println_string("Sample index out of range");
    return 0;
  }
  return 1;
}

void kernel_main() {
  clear_screen();
  println_string("Booting MNIST-OS");

  fpu_init();
  timer_init();
  fat32_init();

  if (!load_file_exact("CONFIG  BIN", &config, sizeof(config))) {
    return;
  }
  if (!validate_config(&config)) {
    return;
  }

  if (!load_file_exact("TRAIN   BIN", train_inputs,
                       config.train_count * config.input_size * sizeof(float))) {
    return;
  }
  if (!load_file_exact("TRNLABELBIN", train_labels,
                       config.train_count * sizeof(uint8_t))) {
    return;
  }
  if (!load_file_exact("TEST    BIN", test_inputs,
                       config.test_count * config.input_size * sizeof(float))) {
    return;
  }
  if (!load_file_exact("TSTLABELBIN", test_labels,
                       config.test_count * sizeof(uint8_t))) {
    return;
  }

  nn_init(&net, &config, W1, b1, W2, b2);

  for (uint32_t i = 0; i < config.train_count; i++) {
    train_order[i] = i;
  }

  uint32_t seed = 0x00C0FFEEU;
  nn_randomize(&net, &seed);

  uint64_t training_start_tsc = timer_read_tsc_start();
  for (uint32_t epoch = 0; epoch < config.epochs; epoch++) {
    shuffle_indices(train_order, config.train_count, &seed);

    float avg_confidence =
        nn_train_epoch(&net, train_inputs, train_labels, train_order,
                       config.train_count, config.learning_rate, hidden_buffer,
                       output_buffer, output_grad, hidden_grad);
    uint32_t train_correct =
        nn_count_correct(&net, train_inputs, train_labels, config.train_count,
                         hidden_buffer, output_buffer);
    uint32_t test_correct =
        nn_count_correct(&net, test_inputs, test_labels, config.test_count,
                         hidden_buffer, output_buffer);

    confidence_history[epoch] = (uint32_t)(avg_confidence * 1000.0f);
    train_accuracy_history[epoch] =
        config.train_count ? (train_correct * 100U) / config.train_count : 0U;
    test_accuracy_history[epoch] =
        config.test_count ? (test_correct * 100U) / config.test_count : 0U;
  }
  uint64_t training_end_tsc = timer_read_tsc_end();
  uint64_t training_ns = timer_elapsed_ns(training_start_tsc, training_end_tsc);

  const float *sample =
      test_inputs + (config.sample_index * config.input_size);
  uint8_t expected_label = test_labels[config.sample_index];

  uint64_t infer_start_tsc = timer_read_tsc_start();
  nn_predict(&net, sample, hidden_buffer, output_buffer);
  uint64_t infer_end_tsc = timer_read_tsc_end();
  uint64_t inference_ns = timer_elapsed_ns(infer_start_tsc, infer_end_tsc);

  clear_screen();
  println_string("Training Summary");
  print_string("Dims: ");
  print_uint32(config.input_size);
  print_string(" -> ");
  print_uint32(config.hidden_size);
  print_string(" -> ");
  println_uint32(config.output_size);

  print_string("Train/Test: ");
  print_uint32(config.train_count);
  print_string(" / ");
  println_uint32(config.test_count);

  print_string("Epochs: ");
  print_uint32(config.epochs);
  print_string("  LR: ");
  print_scaled_thousandths((uint32_t)(config.learning_rate * 1000.0f));
  print_char('\n');

  print_string("Train time: ");
  print_fixed_uint64(uint64_div_u32(training_ns, 1000000U),
                     uint64_mod_u32(training_ns, 1000000U), 6);
  println_string(" ms");
  println_string("");

  print_string("Ep  Conf  Train  Test");
  print_char('\n');
  for (uint32_t epoch = 0; epoch < config.epochs; epoch++) {
    print_uint32(epoch + 1);
    print_string("   ");
    print_scaled_thousandths(confidence_history[epoch]);
    print_string("   ");
    print_uint32(train_accuracy_history[epoch]);
    print_string("%    ");
    print_uint32(test_accuracy_history[epoch]);
    println_string("%");
  }

  println_string("");
  print_string("Expected: ");
  print_uint32(expected_label);
  print_string("  Predicted: ");
  println_uint32((uint32_t)math_argmax(output_buffer, (int)config.output_size));

  print_string("Confidence: ");
  print_scaled_thousandths(
      (uint32_t)(output_buffer[expected_label] * 1000.0f));
  println_string("");

  println_image(sample);

  print_string("Inference: ");
  print_fixed_uint64(uint64_div_u32(inference_ns, 1000000U),
                     uint64_mod_u32(inference_ns, 1000000U), 6);
  print_string(" ms | ");
  print_uint64(inference_ns);
  println_string(" ns");
}
