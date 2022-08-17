#ifndef __ENTITY__
#define __ENTITY__

#include "vector.h"
#include <follia.h>

typedef struct {
    float x, y;
    float dx, dy;

    FL_Texture *sprite;
} entity_t;

#endif