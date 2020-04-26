#include "display.h"

#include <string.h>
#include <stdio.h>
#include "common.h"

#define NUM_BUFFERS 2

struct {
	pixel_t *ptr;
	uint16_t addr, offset;
} buffers[NUM_BUFFERS];
size_t write_index = 0, read_index = 1;
uint32_t block_columns = 1, block_rows = 1;
size_t buffer_length;

void display_configure(uint32_t columns, uint32_t rows) {
	block_columns = columns;
	block_rows = rows;
	buffer_length = SEGMENTS_IN_BLOCK * block_rows * block_columns * sizeof(pixel_t);
}

void display_set_buffer(size_t num, pixel_t *ptr, uint16_t addr, uint16_t offset) {
	if (num < NUM_BUFFERS) {
		buffers[num].ptr = ptr;
		buffers[num].addr = addr;
		buffers[num].offset = offset;
	}
}

#define define_getter(prop, read_write, return_type) \
	return_type display_##read_write##_##prop() {	 \
		return buffers[read_write##_index].prop;	 \
	}

#define define_read_write_getter(prop, return_type) \
	define_getter(prop, read, return_type)			\
	define_getter(prop, write, return_type)			\

define_read_write_getter(ptr, pixel_t*)
define_read_write_getter(addr, uint16_t)
define_read_write_getter(offset, uint16_t)


void display_swap_buffers() {
	size_t _write_index = write_index;
	write_index = read_index;
	read_index = _write_index;
}

pixel_t* display_clear() {
	pixel_t *data = display_write_ptr();
	printf("clearing %d\n", buffer_length);
	memset(data, 0, buffer_length);
	return data;
}


void display_debug() {
	static size_t counter = 0;
	pixel_t *data = display_clear();

	for (int i = 0; i < 256; i++) {
		data[(i + counter) % 256] = i; //(i + counter) % 256;
	}
	counter = (counter + 1) % buffer_length;
	/* data[0] = 255; */
	/* for (int i = 0; i < 256; i+= 3) { */
	/* 	data[i] = i / 3 % 3 * 80 + 10; */
	/* } */
}
