#ifndef __PLAYER__
#define __PLAYER__

typedef struct {
    float x, y;
    float dx, dy;
    float px, py;

    float move;
    int strafe;
    float turn;

    float angle;
} player_t;

#endif