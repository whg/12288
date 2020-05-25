#pragma once

#include <stdint.h>

typedef enum {
	RENDERER_NONE,
	RENDERER_NEW_FRAME,
	RENDERER_BIT_DEPTH,
	RENDERER_ENABLE_TICKS,
} renderer_status_t;

void renderer_init(int cols, int rows);
renderer_status_t renderer_write_frame();
uint32_t renderer_get_value();
void renderer_stop();
void renderer_close();
