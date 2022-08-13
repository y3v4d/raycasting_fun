#ifndef __PLAYER__
#define __PLAYER__

typedef struct {
    float x, y;
    float angle, vangle;

    float dx, dy;

    float move;
} player_t;

#endif