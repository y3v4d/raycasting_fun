#include <follia.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "core/fl_key.h"
#include "debug.h"

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

    vec2f_t mouse = { 0 };

    uint8_t map[16] = {
        1, 2, 1, 2,
        2, 0, 0, 1,
        1, 0, 0, 2,
        2, 1, 2, 1
    };

    player_t player = { 
        .x = 1.5, 
        .y = 1.5,
        .dx = 0,
        .dy = -1,
        .move = 0,
        .strafe = 0,
        .turn = 0
    };

    char column_info[64] = "C: - D: -";
    char player_info_text[128] = "";
    char stats_text[512];

    float plane_x = 0.57;
    float plane_y = 0;

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
            for(int i = 0; i < 20; ++i) {
                if(curr_dist_x < curr_dist_y) {
                    mx += map_step_x;
                    curr_dist_x += d_dist_x;
                    side = 1;
                } else {
                    my += map_step_y;
                    curr_dist_y += d_dist_y;
                    side = 0;
                }

                if(my < 0 || my >= 4 || mx < 0 || mx >= 4) continue;
                if(map[my * 4 + mx] != 0) {
                    mm_points[mm_points_count].x = player.x + r_dir_x * 0.5;
                    mm_points[mm_points_count].y = player.y + r_dir_y * 0.5;

                    mm_points_count++;
                    hit = map[my * 4 + mx];
                    break;
                }
            }

            float distance = 0;
            if(side == 0) distance = curr_dist_y - d_dist_y;
            else distance = curr_dist_x - d_dist_x;

            if(hit) {
                float lineHeight = FL_GetWindowHeight() / distance;

                FL_DrawLine(
                    i, 
                    (PROJECTION_HEIGHT >> 1) - lineHeight / 2, 
                    i, 
                    (PROJECTION_HEIGHT >> 1) + lineHeight / 2, 
                    hit == 1 ? 0xffff00 : 0xff00ff);
            }
            //float x_side_delta = 0;
            //float y_side_delta = 0;
        }

        // === DRAW MINIMAP ===
        const float mm_w = 128, mm_h = 128;
        const int mm_x = FL_GetWindowWidth() - mm_w - 8;
        const int mm_y = 8;
        const float mm_grid = mm_w / 4;

        FL_DrawRect(mm_x, mm_y, mm_w, mm_h, 0x444444, true);

        for(int y = 0; y < 4; ++y) {
            for(int x = 0; x < 4; ++x) {
                if(map[y * 4 + x] != 0) {
                    FL_DrawRect(
                        mm_x + x * mm_grid, 
                        mm_y + y * mm_grid, 
                        mm_grid, mm_grid, 
                        map[y * 4 + x] == 1 ? 0xffff00 : 0xff00ff, 
                        true
                    );
                }
                FL_DrawRect(mm_x + x * mm_grid, mm_y + y * mm_grid, mm_grid, mm_grid, 0, false);
            }
        }

        for(int i = 0; i < mm_points_count; ++i) {
            FL_DrawCircle(mm_x + mm_points[i].x * mm_grid, mm_y + mm_points[i].y * mm_grid, 1, 0x0000ff, true);
        }

        FL_DrawCircle(mm_x + player.x * mm_grid, mm_y + player.y * mm_grid, 2, 0xff0000, true);
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

    FL_FreeFontBDF(knxt);
    FL_Close();

    return 0;
}
