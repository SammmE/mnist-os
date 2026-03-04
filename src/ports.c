#include "ports.h"

uint8_t port_byte_in(uint16_t port) {
  uint8_t result;
  __asm__ __volatile__("inb %%dx, %%al" : "=a"(result) : "d"(port));
  return result;
}

void port_byte_out(uint16_t port, uint8_t data) {
  __asm__ __volatile__("outb %%al, %%dx" : : "a"(data), "d"(port));
}

uint16_t port_word_in(uint16_t port) {
  uint16_t result;
  __asm__ __volatile__("inw %%dx, %%ax" : "=a"(result) : "d"(port));
  return result;
}

void port_word_out(uint16_t port, uint16_t data) {
  __asm__ __volatile__("outw %%ax, %%dx" : : "a"(data), "d"(port));
}
