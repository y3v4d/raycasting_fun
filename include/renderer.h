#ifndef __RENDERER__
#define __RENDERER__

#include <follia.h>
#include <stdint.h>

#include "map.h"
#include "player.h"

#define PROJECTION_WIDTH 640
#define PROJECTION_HEIGHT 480

extern FL_Texture *wall0, *floor0;
extern float z_buffer[PROJECTION_WIDTH];

void r_draw_column(int column, float distance, uint32_t color);
void r_draw_column_textured(int column, float offset, float distance, FL_Texture *texture);

void r_draw_walls(const map_t *map, const player_t *p);
void r_draw_floor(const player_t *p);

#endif