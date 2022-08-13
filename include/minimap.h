#ifndef __MINIMAP__
#define __MINIMAP__

#include "map.h"
#include "player.h"

typedef struct {
    int x, y;
    int w, h;

    const map_t *map;
    const player_t *player;
} minimap_t;

void minimap_draw(const minimap_t *m);

#endif