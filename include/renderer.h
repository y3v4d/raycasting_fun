#ifndef __RENDERER__
#define __RENDERER__

#include <follia.h>
#include <stdint.h>

#define PROJECTION_WIDTH 640
#define PROJECTION_HEIGHT 480
#define GRID_SIZE 32

void r_draw_column(int column, float distance, uint32_t color);
void r_draw_column_textured(int column, int offset, float distance, FL_Texture *texture);

#endif