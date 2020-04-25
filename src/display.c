#include "display.h"

#include <string.h>
#include <stdio.h>
#include "common.h"

#define NUM_BUFFERS 2

struct {
	pixel_t *ptr;
	uint32_t addr;
} buffers[NUM_BUFFERS];
size_t write_index = 0, read_index = 1;
uint32_t block_columns = 1, block_rows = 1;
size_t buffer_length;

void display_configure(uint32_t columns, uint32_t rows) {
	block_columns = columns;
	block_rows = rows;
	buffer_length = SEGMENTS_IN_BLOCK * block_rows * block_columns * sizeof(pixel_t);
}

void display_set_buffer(size_t num, pixel_t *ptr, uint32_t addr) {
	if (num < NUM_BUFFERS) {
		buffers[num].ptr = ptr;
		buffers[num].addr = addr;
	}
}

pixel_t* display_read_data() {
	return buffers[read_index].ptr;
}

pixel_t* display_write_data() {
	return buffers[write_index].ptr;
}

uint32_t display_read_addr() {
	return buffers[read_index].addr;
}

uint32_t display_write_addr() {
	return buffers[write_index].addr;
}

void display_swap_buffers() {
	size_t _write_index = write_index;
	write_index = read_index;
	read_index = _write_index;
}

void display_clear() {
	pixel_t *data = display_write_data();
	memset(data, 0, buffer_length);
}


void display_debug() {
  /* pixel_t *ptr = display_write_data(); */
  /* uint8_t value = 255; */
  /* for (int i = 0; i < 1; i++) { */
  /*   *ptr = 255; */
  /*   ptr++; */
  /* } */
  //ptr[DATA_LEN - 8] = 255;


  /* for (int i = 0; i < ROW_LEN * COMMON_OUTPUTS; i++) { */
  /*   //\*ptr = value--;// * (i % 2); */
  /*   *ptr = 255; */
  /*   ptr += STRIDE; */
  /* } */

  /* ptr = display_write_data(); */
  /* int index; */
  /* for (int i = 0; i < 20; i++) { */
  /*   index = (ROW_LEN - 1) * STRIDE - (STRIDE * i * 2); */
  /*   ptr[index] = 127; */
  /* } */

  /* for (int i = 0; i < 256; i++) { */
  /*   //    ptr[i * STRIDE * 8] = 255; */
  /*   ptr[i * STRIDE] = i; */
  /* } */

  /* index = (ROW_LEN - 1) * STRIDE; */
  /* ptr[index] = 255; */


  /* for (int i = 0; i < ROW_LEN * COMMON_OUTPUTS ; i++) { */
  /*   //    *ptr = value--;// * (i % 2); */
  /*   //    *ptr = 255; //i * 4 % 255; //i % 16; */
  /*     *(ptr + 1) = 255; */
  /*   ptr += STRIDE;// * (i % 5); */
  /* } */


  /* for (int i = 0; i < ROW_LEN * COMMON_OUTPUTS; i++) { */
  /*   *ptr = 0; //i % 16; */
  /*   ptr+= STRIDE; */
  /* } */


  /* ptr = display_write_data(); */
  /* uint32_t last = ((ROW_LEN) * STRIDE) * 10; //COMMON_OUTPUTS ; */
  /* last = ROW_LEN * COMMON_OUTPUTS * STRIDE - 8; */
  /* last -= ROW_LEN * 5 * STRIDE; */
  /*   //  ptr[(last - STRIDE)] = 255; */
  /* for (int i = 0; i < 16; i++) { */
  /*   ptr[last + i * STRIDE] = 255; */
  /* } */


  //  ptr[0] = ptr[8] = 255;

  //  ptr[(ROW_LEN - 1) * STRIDE - (STRIDE * 1)] = 0;

}
