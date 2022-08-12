#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <follia.h>

#include "map.h"

typedef struct {
    float x, y;
} vec2f_t;

typedef struct {
    int x, y;
} vec2i_t;

#define MAX_TEXT 64

int strtoint(const char *str) {
    int i = 0;
    while(*str != 0) {
        if(*str < '0' || *str > '9') {
            i = -1;
            break;
        }

        i = i * 10 + *str - '0';
        str++;
    }

    return i;
}

int main(int argc, char **argv) {
    const char *path = NULL;
    map_t *map = NULL;

    if(argc == 2) {
        path = argv[1];

        map = map_load(path);
        if(!map) {
            fprintf(stderr, "Couldn't open %s\n", path);
            return -1;
        }
    } else if(argc == 4) { // create a new map with specified width and height
        path = argv[1];

        map = (map_t*)malloc(sizeof(map_t));
        map->width = strtoint(argv[2]);
        map->height = strtoint(argv[3]);

        printf("W: %d H: %d\n", map->width, map->height);
        if(map->width < 0 || map->height < 0) {
            free(map);
            return -1;
        }

        map->data = (uint8_t*)malloc(sizeof(uint8_t) * map->width * map->height);
        for(int i = 0; i < map->width * map->height; ++i) {
            map->data[i] = 0;
        }
    } else return 0;

    if(!FL_Initialize(640, 480)) return -1;

    FL_SetTitle("Raycasting Map Editor");

    int grid_size = 32;
    vec2f_t position = { 0, 0 };

    FL_Bool is_moving = false;
    vec2i_t mouse_pos = { 0 };
    vec2i_t mouse_delta = { 0 };
    const float SPEED = 1.2f;

    FL_Event event;
    while(!FL_WindowShouldClose()) {
        while(FL_GetEvent(&event)) {
            if(event.type == FL_EVENT_MOUSE_MOVED) {
                mouse_delta.x = event.mouse.x - mouse_pos.x;
                mouse_delta.y = event.mouse.y - mouse_pos.y;

                mouse_pos.x = event.mouse.x;
                mouse_pos.y = event.mouse.y;
            } else if(event.type == FL_EVENT_MOUSE_PRESSED) {
                if(event.mouse.button == 4) grid_size++;
                else if(event.mouse.button == 5 && grid_size > 0) grid_size--;
                else if(event.mouse.button == 3) {
                    is_moving = true;
                }
            } else if(event.type == FL_EVENT_MOUSE_RELEASED) {
                if(event.mouse.button == 1) {
                    int mx = floorf((mouse_pos.x - position.x) / grid_size), my = floorf((mouse_pos.y - position.y) / grid_size);

                    if(mx < 0 || mx >= map->width || my < 0 || my >= map->height) continue;
                    map->data[my * map->width + mx] = !map->data[my * map->width + mx];
                } else if(event.mouse.button == 3) {
                    is_moving = false;
                }
            }
        }

        if(is_moving) {
            position.x += mouse_delta.x;
            position.y += mouse_delta.y;
        }

        mouse_delta.x = 0;
        mouse_delta.y = 0;

        FL_ClearScreen();
        for(int y = 0; y < map->height; ++y) {
            for(int x = 0; x < map->width; ++x) {
                int rx = floorf(position.x) + x * grid_size, ry = floorf(position.y) + y * grid_size;
                uint32_t color = (map->data[y * map->width + x] == 0 ? 0x6666aa : 0xaaaa66);

                FL_DrawRect(rx, ry, grid_size, grid_size, color, true);
                FL_DrawRect(rx, ry, grid_size, grid_size, 0, false);
            }
        }
        FL_Render();
    }

    map_dump(map, path);
    map_close(map);

    FL_Close();
    return 0;
}