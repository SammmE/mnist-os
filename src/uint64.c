#include "uint64.h"

void uint64_divmod_u32(uint64_t dividend, uint32_t divisor, uint64_t *quotient,
                       uint32_t *remainder) {
  uint64_t q = 0;
  uint64_t r = 0;

  for (int i = 63; i >= 0; i--) {
    r = (r << 1) | ((dividend >> i) & 1ULL);
    if (r >= divisor) {
      r -= divisor;
      q |= (1ULL << i);
    }
  }

  if (quotient) {
    *quotient = q;
  }
  if (remainder) {
    *remainder = (uint32_t)r;
  }
}

uint64_t uint64_div_u32(uint64_t dividend, uint32_t divisor) {
  uint64_t quotient;

  uint64_divmod_u32(dividend, divisor, &quotient, 0);
  return quotient;
}

uint32_t uint64_mod_u32(uint64_t dividend, uint32_t divisor) {
  uint32_t remainder;

  uint64_divmod_u32(dividend, divisor, 0, &remainder);
  return remainder;
}
