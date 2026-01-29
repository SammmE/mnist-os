#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

// Initialize the Programmable Interval Timer (PIT)
void timer_init(uint32_t frequency);

// Get the current tick count
uint32_t timer_get_ticks(void);

// Wait for a specified number of ticks
void timer_wait(uint32_t ticks);

#endif
