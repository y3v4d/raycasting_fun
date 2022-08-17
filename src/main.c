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

FL_Texture *wall0, *floor0;

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

    wall0 = FL_LoadTexture("data/textures/wall0.bmp");
    if(!wall0) {
        FL_Close();
        exit(-1);
    }

    floor0 = FL_LoadTexture("data/textures/floor0.bmp");
    if(!floor0) {
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

        // === DRAW WORLD ===
        r_draw_floor(map, &player);
        r_draw_walls(map, &player);

        // === DRAW MINIMAP ===
        minimap_draw(&minimap);
        
        // === DRAW MOUSE COLUMN ===
        FL_DrawLine(mouse.x, 0, mouse.x, PROJECTION_HEIGHT - 1, 0x0000ff);

        // === DRAW DEBUG TEXT ===
        FL_DrawTextBDF(8, 8, stats_text, 512, PROJECTION_WIDTH - 16, font);
        FL_DrawTextBDF(4, PROJECTION_HEIGHT - 52, player_info_text, 128, 640, font);
        FL_DrawTextBDF(4, PROJECTION_HEIGHT - 28, column_info, 64, 640, font);

        FL_Render();
    }

    map_close(map);
    FL_FreeTexture(floor0);
    FL_FreeTexture(wall0);

    FL_FreeFontBDF(font);
    FL_Close();

    return 0;
}
