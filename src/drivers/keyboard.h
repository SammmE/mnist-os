#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

// Keyboard initialization
void keyboard_init(void);

// Get a character from the keyboard buffer (blocking)
char keyboard_getchar(void);

// Check if a key is available
int keyboard_available(void);

#endif
