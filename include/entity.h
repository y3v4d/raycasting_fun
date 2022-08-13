#ifndef __ENTITY__
#define __ENTITY__

#include <follia.h>

typedef struct {
    float x, y;
    float w, h;

    FL_Texture *texture;
} entity_t;

#endif