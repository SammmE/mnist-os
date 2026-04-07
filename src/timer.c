#include "ports.h"
#include "timer.h"
#include "uint64.h"

#define PIT_COMMAND_PORT 0x43
#define PIT_CHANNEL2_PORT 0x42
#define PIT_SPEAKER_PORT 0x61
#define PIT_BASE_FREQUENCY 1193182ULL

static uint32_t cycles_per_second = 0;

static void pit_wait_ticks(uint16_t reload_value) {
  uint8_t speaker_state = port_byte_in(PIT_SPEAKER_PORT);

  // Disable the speaker, then toggle the gate to restart channel 2 cleanly.
  port_byte_out(PIT_SPEAKER_PORT, speaker_state & ~0x02);
  port_byte_out(PIT_COMMAND_PORT, 0xB0);
  port_byte_out(PIT_CHANNEL2_PORT, (uint8_t)(reload_value & 0xFF));
  port_byte_out(PIT_CHANNEL2_PORT, (uint8_t)(reload_value >> 8));
  port_byte_out(PIT_SPEAKER_PORT, (speaker_state & ~0x02) | 0x01);

  while ((port_byte_in(PIT_SPEAKER_PORT) & 0x20) == 0) {
  }

  port_byte_out(PIT_SPEAKER_PORT, speaker_state);
}

void timer_init() {
  const uint16_t calibration_ticks = (uint16_t)(PIT_BASE_FREQUENCY / 20);
  const uint64_t calibration_ms = 50;
  uint64_t start_tsc = timer_read_tsc_start();

  pit_wait_ticks(calibration_ticks);

  uint64_t end_tsc = timer_read_tsc_end();
  uint64_t elapsed_cycles = end_tsc - start_tsc;

  cycles_per_second =
      (uint32_t)uint64_div_u32(elapsed_cycles * 1000ULL, calibration_ms);
  if (cycles_per_second == 0) {
    cycles_per_second = 1;
  }
}

uint64_t timer_read_tsc() {
  uint32_t lo;
  uint32_t hi;

  __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
  return ((uint64_t)hi << 32) | lo;
}

uint64_t timer_read_tsc_start() {
  uint32_t lo;
  uint32_t hi;

  // Serialize earlier work before taking the start timestamp.
  __asm__ __volatile__("cpuid\n\t"
                       "rdtsc"
                       : "=a"(lo), "=d"(hi)
                       : "a"(0)
                       : "ebx", "ecx");
  return ((uint64_t)hi << 32) | lo;
}

uint64_t timer_read_tsc_end() {
  uint32_t lo;
  uint32_t hi;

  // Serialize earlier work before taking the end timestamp.
  __asm__ __volatile__("cpuid\n\t"
                       "rdtsc"
                       : "=a"(lo), "=d"(hi)
                       : "a"(0)
                       : "ebx", "ecx");
  return ((uint64_t)hi << 32) | lo;
}

uint64_t timer_elapsed_ns(uint64_t start_tsc, uint64_t end_tsc) {
  uint64_t elapsed_cycles = end_tsc - start_tsc;

  if (cycles_per_second == 0) {
    return 0;
  }

  return uint64_div_u32(elapsed_cycles * 1000000000ULL, cycles_per_second);
}
