#ifndef __MAP_H__
#define __MAP_H__

#include <stdint.h>

typedef struct {
    // map data
    uint8_t width, height;
    uint8_t *data;
} map_t;

map_t* map_load(const char *path);
void map_close(map_t *p);

int map_dump(const map_t *p, const char *path);

#endif