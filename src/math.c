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
