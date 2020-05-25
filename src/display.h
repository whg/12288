#pragma once

#include <stdint.h>
#include <stdlib.h>

typedef uint8_t pixel_t;

void display_configure(uint32_t columns, uint32_t rows);
void display_set_buffer(size_t num, pixel_t *ptr, uint16_t addr, uint16_t offset);
void display_swap_buffers();
void display_use_buffer(size_t num);

pixel_t* display_read_ptr();
uint16_t display_read_addr();
uint16_t display_read_offset();

pixel_t* display_write_ptr();
uint16_t display_write_addr();
uint16_t display_write_offset();

pixel_t* display_clear();

void display_cleanup();

void display_debug();
