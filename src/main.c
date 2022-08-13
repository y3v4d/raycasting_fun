#include <follia.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "debug.h"
#include "map.h"
#include "renderer.h"
#include "tables.h"
#include "minimap.h"

float absf(float x) {
    return x < 0 ? -x : x;
}

typedef struct {
    float x, y;
} vec2f_t;

typedef struct {
    int x, y;
} vec2i_t;

int main() {
    if(!FL_Initialize(640, 480))
        exit(-1);

    FL_SetTitle("Raycasting Test");
    FL_SetFrameTime(16.6);
    FL_SetTextColor(0);

    map_t *map = map_load("data/map0.data");
    if(!map) {
        fprintf(stderr, "Error loading map!\n");
        FL_Close();
        exit(-1);
    }

    FL_FontBDF *knxt = FL_LoadFontBDF("data/fonts/knxt.bdf");
    if(!knxt) {
        FL_Close();
        exit(-1);
    }

    FL_Texture *wall0, *wall1, *wall2;
    wall0 = FL_LoadTexture("data/wall0.bmp");
    wall1 = FL_LoadTexture("data/wall1.bmp");
    wall2 = FL_LoadTexture("data/wall2.bmp");
    if(!wall0 || !wall1 || !wall2) {
        FL_Close();
        exit(-1);
    }

    tables_init();

    const float fov = 60.f;
    vec2f_t mouse = { 0 };

    player_t player = { .x = 5 * GRID_SIZE, .y = 5 * GRID_SIZE };
    vec2f_t direction = { 0 };

    char column_info[64] = "C: - D: -";
    char player_pos_text[32];
    char stats_text[512];

    minimap_t minimap = {
        .w = PROJECTION_WIDTH / 8,
        .y = 8,
        .map = map,
        .player = &player
    };

    minimap.x = PROJECTION_WIDTH - minimap.w - 8;
    minimap.h = minimap.w;

    FL_Event event;
    while(!FL_WindowShouldClose()) {
        while(FL_GetEvent(&event)) {
            if(event.type == FL_EVENT_KEY_PRESSED) {
                switch(event.key.code) {
                    case FL_KEY_d: player.vangle = 2; break;
                    case FL_KEY_a: player.vangle = -2; break;
                    case FL_KEY_w: player.move = 0.5; break;
                    case FL_KEY_s: player.move = -0.5; break;
                    default: break;
                }
            } else if(event.type == FL_EVENT_KEY_RELEASED) {
                switch(event.key.code) {
                    case FL_KEY_d: case FL_KEY_a: player.vangle = 0; break;
                    case FL_KEY_w: case FL_KEY_s: player.move = 0; break;
                    default: break;
                }
            } else if(event.type == FL_EVENT_MOUSE_MOVED) {
                mouse.x = event.mouse.x;
                mouse.y = event.mouse.y;
            }
        }

        debug_update_stats();
        debug_print_stats(stats_text, 512);

        // movement
        const float dt = FL_GetDeltaTime();
        player.angle += player.vangle * dt;
        if(player.angle < 0) player.angle += ANGLE_360;
        else if(player.angle >= ANGLE_360) player.angle -= ANGLE_360;

        direction.x = cos_table[(int)player.angle];
        direction.y = -sin_table[(int)player.angle];

        player.x += player.move * direction.x * 0.005f * GRID_SIZE * dt;
        player.y += player.move * direction.y * -0.005f * GRID_SIZE * dt;

        snprintf(player_pos_text, 32, "X: %.2f Y: %.2f A: %.2f", player.x, player.y, player.angle);
      
        FL_ClearScreen();

        // ==== DRAW GAME ====
        /*const int mm_values_max = 1024;
        vec2f_t mm_values[1024] = { 0 };
        int mm_values_size = 0;*/

        const float r = 20;
        const int WALL_HEIGHT = 1;
        const float step = fov / PROJECTION_WIDTH;

        int current = (int)floor(player.angle) - ANGLE_30;
        if(current < 0) current += ANGLE_360;

        vec2i_t player_grid_pos = {
            .x = floor(player.x / GRID_SIZE) * GRID_SIZE,
            .y = floor(player.y / GRID_SIZE) * GRID_SIZE
        };

        for(int i = 0; i < PROJECTION_WIDTH; i++) {
            int v_map_x = -1, v_map_y = -1, h_map_x = -1, h_map_y = -1;
            float v_wall_distance = 999999.f, h_wall_distance = 999999.f;
            int v_offset = -1, h_offset = -1;

            {
                FL_Bool is_down = current >= ANGLE_0 && current <= ANGLE_180;

                const float Xa = GRID_SIZE / tan_table[current];
                const float Ya = is_down ? GRID_SIZE : -GRID_SIZE;

                float Ay = (is_down ? player_grid_pos.y + GRID_SIZE : player_grid_pos.y - 0.0001f);
                float Ax = player.x + (player.y - Ay) / -tan_table[current];

                for(int j = 0; j < r; ++j) {
                    const float distance = absf(player.y - Ay) / sin_table[current];

                    int map_x = floor(Ax / GRID_SIZE), map_y = floor(Ay / GRID_SIZE);

                    if(map_x < 0 || map_x >= map->width || map_y < 0 || map_y >= map->height) break; // ray went out of map
                    if(map->data[map_y * map->height + map_x] != 0) {
                        /*if(mm_values_size < mm_values_max) {
                            mm_values[mm_values_size].x = map_x;
                            mm_values[mm_values_size++].y = map_y;
                        }*/

                        h_offset = absf(Ax - map_x * GRID_SIZE);
                        h_wall_distance = distance / GRID_SIZE;
                        h_map_x = map_x;
                        h_map_y = map_y;

                        break;
                    }

                    Ax += is_down ? Xa : -Xa;
                    Ay += Ya;
                }
            }

            {
                FL_Bool is_right = current >= ANGLE_270 || current <= ANGLE_90;

                const float Xa = is_right ? GRID_SIZE : -GRID_SIZE;
                const float Ya = tan_table[current] * GRID_SIZE;

                float Ax = is_right ? player_grid_pos.x + GRID_SIZE : player_grid_pos.x - 0.0001f;
                float Ay = player.y - (Ax - player.x) * -tan_table[current];

                for(int j = 0; j < r; ++j) {
                    const float distance = absf(player.x - Ax) / cos_table[current];

                    int map_x = floor(Ax / GRID_SIZE), map_y = floor(Ay / GRID_SIZE);
                    if(map_x < 0 || map_x >= map->width || map_y < 0 || map_y >= map->height) break; // ray went out of map

                    if(map->data[map_y * map->width + map_x] != 0) {
                        /*if(mm_values_size < mm_values_max) {
                            mm_values[mm_values_size].x = Ax;
                            mm_values[mm_values_size++].y = Ay;
                        }*/

                        v_offset = absf(Ay - map_y * GRID_SIZE);
                        v_wall_distance = distance / GRID_SIZE;
                        v_map_x = map_x;
                        v_map_y = map_y;

                        break;
                    }

                    Ax += Xa;
                    Ay += is_right ? Ya : -Ya;
                }
            }

            if(absf(h_wall_distance) < absf(v_wall_distance) && absf(h_wall_distance) < r) {
                //draw_column(i, h_wall_distance, h_map_x, h_map_y);
                r_draw_column_textured(i, h_offset, h_wall_distance, wall0);
                if(i == mouse.x) {
                    snprintf(column_info, 64, "C: %d D: %f Offset: %d", i, h_wall_distance, h_offset);
                }
            } else if(absf(v_wall_distance) < absf(h_wall_distance) && absf(v_wall_distance) < r) {
                //draw_column(i, v_wall_distance, v_map_x, v_map_y);
                r_draw_column_textured(i, v_offset, v_wall_distance, wall0);
                if(i == mouse.x) {
                    snprintf(column_info, 64, "C: %d D: %f Offset: %d", i, v_wall_distance, v_offset);
                }
            }

            current += 1;
            if(current >= ANGLE_360) current -= ANGLE_360;
        }

        // ==== DRAW MAP ====
        minimap_draw(&minimap);
        /*for(int i = 0; i < mm_values_size; ++i) {
            vec2f_t *v = &mm_values[i];
            FL_DrawCircle(mm_x + v->x * mm_ratio, mm_y + v->y * mm_ratio, 1, 0xffffff, true);
        }*/

        FL_DrawCircle(mouse.x, mouse.y, 2, 0x0000ff, true);
        FL_DrawLine(mouse.x, 0, mouse.x, PROJECTION_HEIGHT, 0x0000ff);
        
        FL_DrawTextBDF(8, 8, stats_text, 512, FL_GetWindowWidth() - 16, knxt);
        FL_DrawTextBDF(4, PROJECTION_HEIGHT - 28, player_pos_text, 32, 640, knxt);
        FL_DrawTextBDF(4, PROJECTION_HEIGHT - 52, column_info, 64, 640, knxt);

        FL_Render();
    }

    map_close(map);

    FL_FreeTexture(wall0);
    FL_FreeTexture(wall1);
    FL_FreeTexture(wall2);
    FL_FreeFontBDF(knxt);
    FL_Close();

    return 0;
}
