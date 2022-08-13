#ifndef __MINIMAP__
#define __MINIMAP__

#include "map.h"
#include "player.h"
#include "entity.h"

#define MINIMAP_MAX_POINTS 512

typedef struct {
    float x, y;
} vec2f_t;

typedef struct {
    int x, y;
    int w, h;

    const map_t *map;
    const player_t *player;
    const entity_t *entity;

    vec2f_t points[MINIMAP_MAX_POINTS];
    int points_count;
} minimap_t;

void minimap_draw(const minimap_t *m);

#endif