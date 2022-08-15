#include <follia.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "map.h"
#include "debug.h"
#include "entity.h"
#include "minimap.h"
#include "renderer.h"
#include "player.h"
#include "vector.h"

float z_buffer[PROJECTION_WIDTH] = { 0 };

int main() {
    if(!FL_Initialize(PROJECTION_WIDTH, PROJECTION_HEIGHT))
        exit(-1);

    FL_SetTitle("Raycasting Test");
    FL_SetFrameTime(16.6);
    FL_SetTextColor(0);

    FL_FontBDF *font = FL_LoadFontBDF("data/fonts/knxt.bdf");
    if(!font) {
        FL_Close();
        exit(-1);
    }

    FL_Texture *wall0 = FL_LoadTexture("data/wall0.bmp");
    if(!wall0) {
        FL_Close();
        exit(-1);
    }

    FL_Texture *floor = FL_LoadTexture("data/floor0.bmp");
    if(!floor) {
        FL_Close();
        exit(-1);
    }

    map_t *map = map_load("data/map0.data");
    if(!map) {
        fprintf(stderr, "Error loading map!\n");
        FL_Close();
        exit(-1);
    }

    player_t player = { 
        .x = 10, 
        .y = 6,
        .dx = 1,
        .dy = 0,
        .px = 0,
        .py = 0.66,
        .move = 0,
        .strafe = 0,
        .turn = 0
    };

    entity_t entity = {
        .x = 11,
        .y = 6
    };

    minimap_t minimap = {
        .w = 128,
        .h = 128,
        .y = 8,

        .map = map,
        .player = &player,
        .entity = NULL,
        .points_count = 0
    };
    minimap.x = PROJECTION_WIDTH - minimap.w - 8;

    char column_info[64] = "C: - D: -";
    char player_info_text[128] = "";
    char stats_text[512];

    vec2f_t mouse = { 0 };

    FL_Event event;
    while(!FL_WindowShouldClose()) {
        while(FL_GetEvent(&event)) {
            if(event.type == FL_EVENT_KEY_PRESSED) {
                switch(event.key.code) {
                    case FL_KEY_w: player.move = 0.002f; break;
                    case FL_KEY_s: player.move = -0.002f; break;
                    case FL_KEY_d: player.turn = 0.05f; break;
                    case FL_KEY_a: player.turn = -0.05f; break;
                    case FL_KEY_SPACE: player.strafe = 1; break;
                    default: break;
                }
            } else if(event.type == FL_EVENT_KEY_RELEASED) {
                switch(event.key.code) {
                    case FL_KEY_w: case FL_KEY_s: player.move = 0; break;
                    case FL_KEY_a: case FL_KEY_d: player.turn = 0; break;
                    case FL_KEY_SPACE: player.strafe = 0; break;
                    default: break;
                }
            } else if(event.type == FL_EVENT_MOUSE_MOVED) {
                mouse.x = event.mouse.x;
                mouse.y = event.mouse.y;
            }
        }

        // movement
        const float dt = FL_GetDeltaTime();

        const float oldDx = player.dx;
        player.dx = player.dx * cos(player.turn) - player.dy * sin(player.turn); 
        player.dy = player.dy * cos(player.turn) + oldDx * sin(player.turn);

        const float oldPlaneX = player.px;
        player.px = player.px * cos(player.turn) - player.py * sin(player.turn);
        player.py = player.py * cos(player.turn) + oldPlaneX * sin(player.turn);

        if(player.strafe) {
            player.x += player.move * -player.dy * dt;
            player.y += player.move * player.dx * dt;
        } else {
            player.x += player.move * player.dx * dt;
            player.y += player.move * player.dy * dt;
        }

        snprintf(
            player_info_text, 128, 
            "Pos (%.2f %.2f) Dir (%.4f %.4f) Slope(%.4f)", 
            player.x, player.y, 
            player.dx, player.dy, 
            player.dy / player.dx
        );

        debug_update_stats();
        debug_print_stats(stats_text, 512);
      
        FL_ClearScreen();

        // === DRAW FLOOR ===
        for(int i = PROJECTION_HEIGHT >> 1; i < PROJECTION_HEIGHT; ++i) {
            int p = (PROJECTION_HEIGHT >> 1) - i;

            float ray_dir0_x = player.dx - player.px;
            float ray_dir0_y = player.dy - player.py;
            float ray_dir1_x = player.dx + player.px;
            float ray_dir1_y = player.dy + player.py;

            const float player_z = PROJECTION_HEIGHT >> 1;

            float r = -player_z / p;

            float floorStepX = r * (ray_dir1_x - ray_dir0_x) / PROJECTION_WIDTH;
            float floorStepY = r * (ray_dir1_y - ray_dir0_y) / PROJECTION_WIDTH;

            float floorX = player.x + ray_dir0_x * r;
            float floorY = player.y + ray_dir0_y * r;

            if(mouse.y == i) {
                snprintf(column_info, 64, "S: %d P: %d R: %f Floor (%.2f %.2f)\n", i, p, r, floorX, floorY);
            }

            for(int x = 0; x < PROJECTION_WIDTH; ++x) {
                int mx = floorf(floorX), my = floorf(floorY);

                int tx = (int)(floor->width * (floorX - mx)) & (floor->width - 1);
                int ty = (int)(floor->height * (floorY - my)) & (floor->height - 1);

                floorX += floorStepX;
                floorY += floorStepY;

                FL_DrawPoint(x, i, floor->data[ty * floor->width + tx]);
            }
        }

        // === DRAW WALLS ===
        for(int i = 0; i < PROJECTION_WIDTH; ++i) {
            int mx = floorf(player.x), my = floorf(player.y); // map coordinates of the player
            
            float cam_x = (float)i * 2 / PROJECTION_WIDTH - 1;

            float ray_dx = player.dx + player.px * cam_x;
            float ray_dy = player.dy + player.py * cam_x;

            // only relative distance matter, not the exact one
            float delta_x = absf(1.f / ray_dx);
            float delta_y = absf(1.f / ray_dy);

            float curr_dx, curr_dy;

            int map_step_x = 0, map_step_y = 0;
            if(ray_dx >= 0) {
                map_step_x = 1;
                curr_dx = (mx + 1 - player.x) * delta_x;
            } else {
                map_step_x = -1;
                curr_dx = (player.x - mx) * delta_x;
            }

            if(ray_dy >= 0) {
                map_step_y = 1;
                curr_dy = (my + 1 - player.y) * delta_y;
            } else {
                map_step_y = -1;
                curr_dy = (player.y - my) * delta_y;
            }

            int side = 0; // 0 - horizontal 1 - vertical
            int hit = 0;
            for(int i = 0; i < 50; ++i) {
                if(curr_dx < curr_dy) {
                    mx += map_step_x;
                    curr_dx += delta_x;
                    side = 1;
                } else {
                    my += map_step_y;
                    curr_dy += delta_y;
                    side = 0;
                }

                if(my < 0 || my >= map->height || mx < 0 || mx >= map->width) continue;
                if(map->data[my * map->width + mx] != 0) {
                    hit = map->data[my * map->width + mx];
                    break;
                }
            }

            float distance = (side == 0 ? curr_dy - delta_y : curr_dx - delta_x);
            float offset;
            if(side == 0) {
                offset = player.x + distance * ray_dx;
                offset -= floorf(offset);
            } else {
                offset = player.y + distance * ray_dy;
                offset -= floorf(offset);
            }

            if(hit) {
                float lineHeight = (float)PROJECTION_HEIGHT / distance;

                r_draw_column_textured(i, offset, distance, wall0);
                z_buffer[i] = distance;
            } else {
                z_buffer[i] = 1e30;
            }

            if(mouse.x == i) {
                //snprintf(column_info, 64, "C: %d Z: %f", i, z_buffer[i]);
            }
        }

         // === DRAW ENTITIES ===
        {
            vec2f_t Z = {
                .x = entity.x - player.x,
                .y = entity.y - player.y
            };

            const float w = 1.f / -(player.px * player.dy - player.py * player.dx);

            float rx = w * (Z.x * player.dy - Z.y * player.dx);
            float t = w * (Z.x * player.py - Z.y * player.px);

            float x = (PROJECTION_WIDTH >> 1) * rx / t;

            float half_screen = PROJECTION_HEIGHT >> 1;
            float size = (float)PROJECTION_HEIGHT / t;

            if(t > 0) {
                for(int i = floorf(x - size / 2); i < floorf(x + size / 2); ++i) {
                    int col = (PROJECTION_WIDTH >> 1) - i;
                    if(col < 0 || col >= PROJECTION_WIDTH) continue;

                    if(t < z_buffer[col]) {
                        //FL_DrawLine((PROJECTION_WIDTH >> 1) - i, half_screen - size / 2, (PROJECTION_WIDTH >> 1) - i, half_screen + size / 2, 0xffff00);
                    }
                }
            }
        }

        minimap_draw(&minimap);
        FL_DrawLine(mouse.x, 0, mouse.x, FL_GetWindowHeight() - 1, 0x0000ff);

        // === Draw info text ===
        FL_DrawTextBDF(8, 8, stats_text, 512, FL_GetWindowWidth() - 16, font);
        FL_DrawTextBDF(4, PROJECTION_HEIGHT - 52, player_info_text, 128, 640, font);
        FL_DrawTextBDF(4, PROJECTION_HEIGHT - 28, column_info, 64, 640, font);

        FL_Render();
    }

    map_close(map);
    FL_FreeTexture(floor);
    FL_FreeTexture(wall0);

    FL_FreeFontBDF(font);
    FL_Close();

    return 0;
}
