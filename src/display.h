#pragma once

#include <stdint.h>
#include <stdlib.h>

typedef uint8_t pixel_t;

void display_configure(uint32_t columns, uint32_t rows);
void display_set_buffer(size_t num, pixel_t *ptr, uint32_t offset);
void display_swap_buffers();

pixel_t* display_read_data();
pixel_t* display_write_data();

uint32_t display_read_addr();
uint32_t display_write_addr();


void display_clear();

void display_cleanup();

void display_debug();
