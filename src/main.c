#include <follia.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "map.h"
#include "debug.h"
#include "entity.h"

#define PI 3.1415926535

#define FOV 60
#define GRID_SIZE 32
#define PROJECTION_WIDTH 640
#define PROJECTION_HEIGHT 480

float absf(float x) {
    return x < 0 ? -x : x;
}

typedef struct {
    float x, y;
} vec2f_t;

typedef struct {
    int x, y;
} vec2i_t;

typedef struct {
    float x, y;
    float dx, dy;

    float move;
    int strafe;
    float turn;

    float angle;
} player_t;

void draw_column(int column, float distance, uint32_t color) {
    //const float proj_distance = (float)PROJECTION_WIDTH / 2 / tan_table[ANGLE_30];
    //const uint32_t shade = (distance == 0 ? 1 : min(255.f / absf(distance) * 2, 255));
    const float half_height = (float)FL_GetWindowHeight() / distance / 2;//(float)GRID_SIZE / cdistance * proj_distance / 2;

    FL_DrawLine(column, (PROJECTION_HEIGHT >> 1) - half_height, column, (PROJECTION_HEIGHT >> 1) + half_height, color);
}

void draw_column_textured(int column, int offset, float distance, FL_Texture *texture) {
    const float height = (float)FL_GetWindowHeight() / distance;

    if(height < PROJECTION_HEIGHT) {
        const float half_height = height / 2;

        const float tex_step = GRID_SIZE / height;
        float tex_current = 0;
        float brightness = 1.f / absf(distance);

        uint32_t *p = texture->data + offset;
        for(int i = floor((PROJECTION_HEIGHT >> 1) - half_height); i < floor((PROJECTION_HEIGHT >> 1) + half_height); ++i) {
            uint32_t color = *(p + (int)(floor(tex_current)) * texture->width);
            uint8_t r = color >> 16, g = color >> 8, b = color;

            r *= brightness;
            g *= brightness;
            b *= brightness;

            FL_DrawPoint(column, i, (uint32_t)(r << 16 | g << 8 | b));

            tex_current += tex_step;
        }

        FL_DrawLine(column, 0, column, (PROJECTION_HEIGHT >> 1) - half_height - 1, 0x666666);
        FL_DrawLine(column, (PROJECTION_HEIGHT >> 1) + half_height, column, PROJECTION_HEIGHT - 1, 0x888888);
    } else {
        const float tex_step = GRID_SIZE / height;
        
        float diff = height - PROJECTION_HEIGHT;
        float tex_current = diff / 2 * tex_step;

        uint32_t *p = texture->data + offset;
        for(int i = 0; i < PROJECTION_HEIGHT; ++i) {
            FL_DrawPoint(column, i, *(p + (int)(floor(tex_current) * texture->width)));

            tex_current += tex_step;
        }
    }
}

int main() {
    if(!FL_Initialize(640, 480))
        exit(-1);

    FL_SetTitle("Raycasting Test");
    FL_SetFrameTime(16.6);
    FL_SetTextColor(0);

    FL_FontBDF *knxt = FL_LoadFontBDF("data/fonts/knxt.bdf");
    if(!knxt) {
        FL_Close();
        exit(-1);
    }

    FL_Texture *wall0 = FL_LoadTexture("data/wall0.bmp");
    if(!wall0) {
        FL_Close();
        exit(-1);
    }

    map_t *map = map_load("data/map0.data");
    if(!map) {
        fprintf(stderr, "Error loading map!\n");
        FL_Close();
        exit(-1);
    }

    vec2f_t mouse = { 0 };

    player_t player = { 
        .x = 10, 
        .y = 5,
        .dx = 1,
        .dy = 0,
        .move = 0,
        .strafe = 0,
        .turn = 0
    };

    entity_t entity = {
        .x = 11,
        .y = 5
    };

    char column_info[64] = "C: - D: -";
    char player_info_text[128] = "";
    char stats_text[512];

    float plane_x = 0;
    float plane_y = 0.66;

    FL_Event event;
    while(!FL_WindowShouldClose()) {
        while(FL_GetEvent(&event)) {
            if(event.type == FL_EVENT_KEY_PRESSED) {
                switch(event.key.code) {
                    case FL_KEY_w: player.move = 0.001f; break;
                    case FL_KEY_s: player.move = -0.001f; break;
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

        const float oldPlaneX = plane_x;
        plane_x = plane_x * cos(player.turn) - plane_y * sin(player.turn);
        plane_y = plane_y * cos(player.turn) + oldPlaneX * sin(player.turn);
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

        const int mm_points_max = 1024;
        vec2f_t mm_points[mm_points_max];
        int mm_points_count = 0;

        // === DRAW WALLS ===
        for(int i = 0; i < PROJECTION_WIDTH; ++i) {
            int mx = floorf(player.x), my = floorf(player.y); // map coordinates of the player
            
            float cam_x = (float)i * 2 / PROJECTION_WIDTH - 1;

            if(i == 0 || i == 320 || i == PROJECTION_WIDTH - 1) {
                //printf("CamX[%d] %f\n", i, cam_x);
            }

            float r_dir_x = player.dx + plane_x * cam_x;
            float r_dir_y = player.dy + plane_y * cam_x;

            // only relative distance matter, not the exact one
            float d_dist_x = absf(1.f / r_dir_x);
            float d_dist_y = absf(1.f / r_dir_y);

            float curr_dist_y = 0;
            float curr_dist_x = 0;

            int map_step_x = 0, map_step_y = 0;
            if(r_dir_x >= 0) {
                map_step_x = 1;
                curr_dist_x = (mx + 1 - player.x) * d_dist_x;
            } else {
                map_step_x = -1;
                curr_dist_x = (player.x - mx) * d_dist_x;
            }

            if(r_dir_y >= 0) {
                map_step_y = 1;
                curr_dist_y = (my + 1 - player.y) * d_dist_y;
            } else {
                map_step_y = -1;
                curr_dist_y = (player.y - my) * d_dist_y;
            }

            int side = 0; // 0 - horizontal 1 - vertical
            int hit = 0;
            for(int i = 0; i < 50; ++i) {
                if(curr_dist_x < curr_dist_y) {
                    mx += map_step_x;
                    curr_dist_x += d_dist_x;
                    side = 1;
                } else {
                    my += map_step_y;
                    curr_dist_y += d_dist_y;
                    side = 0;
                }

                if(my < 0 || my >= map->height || mx < 0 || mx >= map->width) continue;
                if(map->data[my * map->width + mx] != 0) {
                    /*mm_points[mm_points_count].x = player.x + r_dir_x * 0.5;
                    mm_points[mm_points_count].y = player.y + r_dir_y * 0.5;

                    mm_points_count++;*/
                    hit = map->data[my * map->width + mx];
                    break;
                }
            }

            float distance = 0;
            if(side == 0) distance = curr_dist_y - d_dist_y;
            else distance = curr_dist_x - d_dist_x;

            float offset = 0;
            if(side == 0) {
                offset = player.x + distance * r_dir_x;
                offset -= floorf(offset);
                offset *= GRID_SIZE;
                //if(i == 320) printf("Offset: %f\n", offset);
            } else {
                offset = player.y + distance * r_dir_y;
                offset -= floorf(offset);
                offset *= GRID_SIZE;
                //if(i == 320) printf("Offset: %f\n", offset);
            }

            if(hit) {
                float lineHeight = FL_GetWindowHeight() / distance;

                draw_column_textured(i, offset, distance, wall0);
            }
        }

         // === DRAW ENTITIES ===
        {
            vec2f_t Z = {
                .x = entity.x - player.x,
                .y = entity.y - player.y
            };

            const float w = 1.f / -(plane_x * player.dy - plane_y * player.dx);

            float rx = w * (Z.x * player.dy - Z.y * player.dx);
            float t = w * (Z.x * plane_y - Z.y * plane_x);

            printf("T: %f W: %f Rx = %f\n", t, w, rx);

            float x = (PROJECTION_WIDTH >> 1) * rx / t;

            float half_screen = PROJECTION_HEIGHT >> 1;
            float height = (float)PROJECTION_HEIGHT / t;

            if(t > 0) {
                FL_DrawLine((PROJECTION_WIDTH >> 1) - x, half_screen - height / 2, (PROJECTION_WIDTH >> 1) - x, half_screen + height / 2, 0xffff00);
            }
        }

        // === DRAW MINIMAP ===
        const float mm_w = 128, mm_h = 128;
        const int mm_x = FL_GetWindowWidth() - mm_w - 8;
        const int mm_y = 8;
        const float mm_grid = mm_w / map->width;

        FL_DrawRect(mm_x, mm_y, mm_w, mm_h, 0xffffff, true);

        for(int y = 0; y < map->width; ++y) {
            for(int x = 0; x < map->height; ++x) {
                if(map->data[y * map->width + x] != 0) {
                    FL_DrawRect(
                        mm_x + x * mm_grid, 
                        mm_y + y * mm_grid, 
                        mm_grid, mm_grid, 
                        map->data[y * map->width + x] == 1 ? 0xffff00 : 0xff00ff, 
                        true
                    );
                }
                //FL_DrawRect(mm_x + x * mm_grid, mm_y + y * mm_grid, mm_grid, mm_grid, 0, false);
            }
        }

        for(int i = 0; i < mm_points_count; ++i) {
            FL_DrawCircle(mm_x + mm_points[i].x * mm_grid, mm_y + mm_points[i].y * mm_grid, 1, 0x0000ff, true);
        }

        FL_DrawCircle(mm_x + player.x * mm_grid, mm_y + player.y * mm_grid, 3, 0xff0000, true);
        FL_DrawCircle(mm_x + entity.x * mm_grid, mm_y + entity.y * mm_grid, 3, 0x0000ff, true);
        FL_DrawLine(
            mm_x + player.x * mm_grid,
            mm_y + player.y * mm_grid,
            mm_x + (player.x + 2 * player.dx) * mm_grid,
            mm_y + (player.y + 2 * player.dy) * mm_grid,
            0xff0000 
        );

        FL_DrawTextBDF(8, 8, stats_text, 512, FL_GetWindowWidth() - 16, knxt);
        FL_DrawTextBDF(4, PROJECTION_HEIGHT - 52, player_info_text, 128, 640, knxt);
        //FL_DrawTextBDF(4, PROJECTION_HEIGHT - 52, column_info, 64, 640, knxt);

        FL_Render();
    }

    map_close(map);
    FL_FreeTexture(wall0);

    FL_FreeFontBDF(knxt);
    FL_Close();

    return 0;
}
