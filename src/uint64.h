#ifndef UINT64_H
#define UINT64_H

#include "types.h"

uint64_t uint64_div_u32(uint64_t dividend, uint32_t divisor);
uint32_t uint64_mod_u32(uint64_t dividend, uint32_t divisor);
void uint64_divmod_u32(uint64_t dividend, uint32_t divisor, uint64_t *quotient,
                       uint32_t *remainder);

#endif // UINT64_H
