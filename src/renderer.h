#pragma once

typedef enum {
	RENDERER_NONE,
	RENDERER_NEW_FRAME,
} renderer_status_t;

void renderer_init(int cols, int rows);
renderer_status_t renderer_write_frame();
void renderer_stop();
void renderer_close();
