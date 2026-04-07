#ifndef VGA_H
#define VGA_H

#include "types.h"

#define VGA_ADDRESS 0xB8000
#define WHITE_ON_BLACK 0x0F

void clear_screen();
void print_char(char c);
void print_string(const char *str);
void println_string(const char *str);
void print_uint32(uint32_t value);
void println_uint32(uint32_t value);
void print_uint64(uint64_t value);
void println_uint64(uint64_t value);
void print_fixed_uint64(uint64_t whole, uint64_t fraction, int digits);
void print_hex_byte(uint8_t byte);
void println_hex_byte(uint8_t byte);
void print_hex(uint32_t value);
void println_hex(uint32_t value);
void print_image(const float *image);
void println_image(const float *image);

#endif // !VGA_H
