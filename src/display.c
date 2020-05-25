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

void display_use_buffer(size_t num) {
  write_index = num;
  read_index = num;
}

pixel_t* display_clear() {
	pixel_t *data = display_write_ptr();
	memset(data, 0, buffer_length);
	return data;
}

#define NUM_TESTS 9
int numbers[NUM_TESTS][8] = {
					  //G  D  E  C  H  F  A  B
					  { 0, 0, 0, 0, 0, 0, 0, 0 },
					  { 0, 0, 0, 0, 0, 0, 1, 0 },
					  { 0, 0, 0, 0, 0, 0, 0, 1 },
					  { 0, 0, 0, 1, 0, 0, 0, 0 },
					  { 0, 1, 0, 0, 0, 0, 0, 0 },
					  { 0, 0, 1, 0, 0, 0, 0, 0 },
					  { 0, 0, 0, 0, 0, 1, 0, 0 },
					  { 1, 0, 0, 0, 0, 0, 0, 0 },
					  { 0, 0, 0, 0, 1, 0, 0, 0 },
};

void display_debug() {
	static size_t counter = 0;
	pixel_t *data = display_clear();
	size_t last = buffer_length;

	//	counter = 5;
	/* for (int i = 0; i < 32 * 8; i++) { */
	/*   if (i % 2) { */
	/*     int cc = counter; */
	/*     if (cc == 0) cc = 2; */
	/*     else if (cc == 2) cc = 0; */
	/*     else if (cc == 3) cc = 7; */
	/*     else if (cc == 4) cc = 6; */
	/*     else if (cc == 6) cc = 4; */
	/*     else if (cc == 7) cc = 3; */
	/*     for (int r = 0; r < 6; r++) { */
	/*       data[buffer_length - 1 - (r + 6 * cc) - 48 * i] = 1; */
	/*     } */
	/*   } else { */
	/*     for (int r = 0; r < 6; r++) { */
	/*       data[buffer_length - 1 - (r + 6 * counter) - 48 * i] = 1; */
	/*     } */
	/*   } */
	/* } */

	data[buffer_length - 1 - 5] = counter;
	
	counter = (counter + 1) % 256;
	return;

	int repeat = 4;
	int n = (NUM_TESTS * 4) + 5 * repeat;
	if (counter < NUM_TESTS * 4) {
	  for (int i = 0; i < 8; i++) {
	    data[(7 - i) + (counter / NUM_TESTS * 32) % 128] = numbers[counter % NUM_TESTS][i] * 127;
	  }
	} else {
	  int nn = counter - (NUM_TESTS * 4);
	  int k = nn / repeat;
	  if (k < 4) {
	    for (int i = 0; i < 8; i++) {
	      data[(7 - i) + (nn / repeat * 32) % 128] = 127;
	    }
	  } else {
	    for (int j = 0; j < 4; j++) {
	      for (int i = 0; i < 8; i++) {
		data[(7 - i) + (j * 32) % 128] = 127;
	      }
	    }
	  }
	}
	for (int i = 0; i < buffer_length; i++) {
	  printf("%d ", data[i]);
	}
	printf("\n");
	counter = (counter + 1) % n;
}
