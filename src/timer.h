#ifndef TIMER_H
#define TIMER_H

#include "types.h"

void timer_init();
uint64_t timer_read_tsc();
uint64_t timer_read_tsc_start();
uint64_t timer_read_tsc_end();
uint64_t timer_elapsed_ns(uint64_t start_tsc, uint64_t end_tsc);

#endif // TIMER_H
