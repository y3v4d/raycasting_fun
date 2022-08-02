#include <follia.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "map.h"

#define PROJECTION_WIDTH 640
#define PROJECTION_HEIGHT 480
#define PI 3.14159

float absf(float x) {
    return x < 0 ? -x : x;
}

typedef struct {
    float x, y;
} vec2f_t;

typedef struct {
    float x, y;
    float angle, vangle;

    float move;
} player_t;

const float fov = 60.f;

vec2f_t mouse = { 0 };
char column_info[64] = "C: - D: -";
const float grid_size = 32;

FL_Texture *wall0, *wall1, *wall2;

uint32_t make_color(uint8_t r, uint8_t g, uint8_t b) {
    return (uint32_t)((uint8_t)r << 16 | (uint8_t)g << 8 | (uint8_t)b);
}

const int ANGLE_0 = 0;
const int ANGLE_60 = PROJECTION_WIDTH;
const int ANGLE_30 = ANGLE_60 / 2;
const int ANGLE_15 = ANGLE_30 / 2;
const int ANGLE_90 = ANGLE_30 * 3;
const int ANGLE_180 = ANGLE_60 * 3;
const int ANGLE_270 = ANGLE_90 * 3;
const int ANGLE_360 = ANGLE_180 * 2;

float tan_table[3840]; // ANGLE_360 can't seem to work without warnings...
float cos_table[3840];
float sin_table[3840];

float arctorad(float a) {
    return a * PI / ANGLE_180;
}

void draw_column(int column, float distance, int map_x, int map_y) {
    if(column == mouse.x) {
        snprintf(column_info, 64, "C: %d D: %f", column, distance);
    }

    const uint32_t shade = (distance == 0 ? 1 : min(255.f / absf(distance) * 2, 255));
    const uint32_t color = 0;//map[map_y * map_width + map_x] == 1 ? make_color(shade, shade, 0) : make_color(shade, 0, shade);
    
    const float fish = (float)column / PROJECTION_WIDTH * ANGLE_60 - ANGLE_30;
    const float cdistance = absf(distance * cos(arctorad(fish)));
    const float half_height = (float)PROJECTION_HEIGHT / cdistance / 2;

    FL_DrawLine(column, (PROJECTION_HEIGHT >> 1) - half_height, column, (PROJECTION_HEIGHT >> 1) + half_height, color);
}

void draw_textured_column(int column, int offset, float distance, FL_Texture *texture) {
    if(column == mouse.x) {
        snprintf(column_info, 64, "C: %d D: %f Offset: %d", column, distance, offset);
    }

    /*const int tile = map[map_y * map_width + map_x];
    if(tile == 1) tex = wall0;
    else if(tile == 3) tex = wall1;
    else if(tile == 4) tex = wall2;
    else return;*/
    
    const float fish = (float)column / PROJECTION_WIDTH * ANGLE_60 - ANGLE_30;
    const float cdistance = absf(distance * cos(arctorad(fish)));
    const float height = (float)PROJECTION_HEIGHT / cdistance;

    if(height < PROJECTION_HEIGHT) {
        const float half_height = height / 2;

        const float tex_step = grid_size / height;
        float tex_current = 0;

        uint32_t *p = texture->data + offset;
        for(int i = floor((PROJECTION_HEIGHT >> 1) - half_height); i < floor((PROJECTION_HEIGHT >> 1) + half_height); ++i) {
            uint32_t color = *(p + (int)(floor(tex_current) * texture->width));
            uint8_t r = color >> 16, g = color >> 8, b = color;

            float brightness = 1.f / absf(cdistance);
            r *= brightness;
            g *= brightness;
            b *= brightness;

            FL_DrawPoint(column, i, (uint32_t)(r << 16 | g << 8 | b));

            tex_current += tex_step;
        }

        FL_DrawLine(column, 0, column, floor((PROJECTION_HEIGHT >> 1) - half_height) - 1, 0x666666);
    } else {
        const float tex_step = grid_size / height;
        
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

    wall0 = FL_LoadTexture("data/wall0.bmp");
    wall1 = FL_LoadTexture("data/wall1.bmp");
    wall2 = FL_LoadTexture("data/wall2.bmp");
    if(!wall0 || !wall1 || !wall2) {
        FL_Close();
        exit(-1);
    }

    player_t player = { .x = 5 * grid_size, .y = 5 * grid_size };
    vec2f_t direction = { 0 };

    for(int i = 0; i < ANGLE_360; ++i) {
        tan_table[i] = tan(arctorad(i));
        sin_table[i] = sin(arctorad(i));
        cos_table[i] = cos(arctorad(i));
    }

    FL_Timer fps_timer = { 0 };
    char player_pos_text[32];
    char fps_text[16] = "FPS: -";

    FL_Bool first = true;

    FL_Event event;
    while(!FL_WindowShouldClose()) {
        while(FL_GetEvent(&event)) {
            if(event.type == FL_EVENT_KEY_PRESSED) {
                switch(event.key.code) {
                    case FL_KEY_d: player.vangle = 2; break;
                    case FL_KEY_a: player.vangle = -2; break;
                    case FL_KEY_w: player.move = 1; break;
                    case FL_KEY_s: player.move = -1; break;
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

        const float dt = FL_GetDeltaTime();

        // movement
        player.angle += player.vangle * dt;
        if(player.angle < 0) player.angle += ANGLE_360;
        else if(player.angle >= ANGLE_360) player.angle -= ANGLE_360;

        direction.x = cos_table[(int)player.angle];
        direction.y = -sin_table[(int)player.angle];

        player.x += player.move * direction.x * 0.005f * grid_size * dt;
        player.y += player.move * direction.y * -0.005f * grid_size * dt;

        snprintf(player_pos_text, 32, "X: %.2f Y: %.2f A: %.2f", player.x, player.y, player.angle);

        // display fps on screen
        FL_StopTimer(&fps_timer);
        if(fps_timer.delta >= 500) {
            snprintf(fps_text, 16, "FPS: %.2f", 1000.f / dt);

            FL_StartTimer(&fps_timer);
        }
      
        FL_ClearScreen();

        // ==== DRAW GAME ====
        const int mm_values_max = 1024;
        vec2f_t mm_values[1024] = { 0 };
        int mm_values_size = 0;

        const float r = 20;
        const int WALL_HEIGHT = 1;
        const float step = fov / PROJECTION_WIDTH;

        int current = (int)floor(player.angle) - ANGLE_30;
        if(current < 0) current += ANGLE_360;

        for(int i = 0; i < PROJECTION_WIDTH; i++) {
            int v_map_x = -1, v_map_y = -1, h_map_x = -1, h_map_y = -1;
            float v_wall_distance = 999999.f, h_wall_distance = 999999.f;
            int v_offset = -1, h_offset = -1;

            {
                FL_Bool is_down = current >= ANGLE_0 && current <= ANGLE_180;

                const float Xa = grid_size / tan_table[current];
                const float Ya = is_down ? grid_size : -grid_size;

                float Ay = (is_down ? floor(player.y / grid_size) * grid_size + grid_size : floor(player.y / grid_size) * grid_size - 0.0001f);
                float Ax = player.x + (player.y - Ay) / -tan_table[current];

                for(int j = 0; j < r; ++j) {
                    const float distance = absf(player.y - Ay) / sin_table[current];

                    int map_x = floor(Ax / grid_size), map_y = floor(Ay / grid_size);

                    if(map_x < 0 || map_x >= map->width || map_y < 0 || map_y >= map->height) break; // ray went out of map
                    if(map->data[map_y * map->height + map_x] != 0) {
                        if(mm_values_size < mm_values_max) {
                            mm_values[mm_values_size].x = map_x;
                            mm_values[mm_values_size++].y = map_y;
                        }

                        h_offset = absf(Ax - map_x * grid_size);
                        h_wall_distance = distance / grid_size;
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

                const float Xa = is_right ? grid_size : -grid_size;
                const float Ya = tan_table[current] * grid_size;

                float Ax = is_right ? floor(player.x / grid_size) * grid_size + grid_size : floor(player.x / grid_size) * grid_size - 0.0001f;
                float Ay = player.y - (Ax - player.x) * -tan_table[current];

                for(int j = 0; j < r; ++j) {
                    const float distance = absf(player.x - Ax) / cos_table[current];

                    int map_x = floor(Ax / grid_size), map_y = floor(Ay / grid_size);
                    if(map_x < 0 || map_x >= map->width || map_y < 0 || map_y >= map->height) break; // ray went out of map

                    if(map->data[map_y * map->width + map_x] != 0) {
                        if(mm_values_size < mm_values_max) {
                            mm_values[mm_values_size].x = Ax;
                            mm_values[mm_values_size++].y = Ay;
                        }

                        v_offset = absf(Ay - map_y * grid_size);
                        v_wall_distance = distance / grid_size;
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
                draw_textured_column(i, h_offset, h_wall_distance, wall0);
            } else if(absf(v_wall_distance) < absf(h_wall_distance) && absf(v_wall_distance) < r) {
                //draw_column(i, v_wall_distance, v_map_x, v_map_y);
                draw_textured_column(i, v_offset, v_wall_distance, wall0);
            }

            current += 1;
            if(current >= ANGLE_360) current -= ANGLE_360;
        }

        // ==== DRAW MAP ====
        const int mm_w = PROJECTION_WIDTH / 8, mm_h = PROJECTION_WIDTH / 8;
        const int mm_x = PROJECTION_WIDTH - mm_w - 8, mm_y = 8;
        const float mm_ratio = (float)mm_w / map->width;

        // background
        FL_DrawRect(mm_x, mm_y, mm_w, mm_h, 0x4444AA, true);
        FL_DrawRect(mm_x, mm_y, mm_w, mm_h, 0, false);

        for(int y = 0; y < map->height; ++y) {
            for(int x = 0; x < map->width; ++x) {
                int sx = mm_x + mm_ratio * x, sy = mm_y + mm_ratio * y;

                int tile = map->data[y * map->width + x];
                if(tile != 0) {
                    FL_DrawRect(sx, sy, mm_ratio, mm_ratio, 0xFFFF00, true);
                    FL_DrawRect(mm_x + mm_ratio * x, mm_y + mm_ratio * y, mm_ratio, mm_ratio, 0, false);
                }

                //FL_DrawRect(mm_x + mm_ratio * x, mm_y + mm_ratio * y, mm_ratio, mm_ratio, 0, false);
            }
        }

        for(int i = 0; i < mm_values_size; ++i) {
            vec2f_t *v = &mm_values[i];
            FL_DrawCircle(mm_x + v->x * mm_ratio, mm_y + v->y * mm_ratio, 1, 0xffffff, true);
        }

        // player
        FL_DrawCircle(mm_x + player.x / grid_size * mm_ratio, mm_y + player.y / grid_size * mm_ratio, 2, 0xFF0000, true);

        FL_DrawCircle(mouse.x, mouse.y, 2, 0x0000ff, true);
        FL_DrawLine(mouse.x, 0, mouse.x, PROJECTION_HEIGHT, 0x0000ff);

        FL_SetTextColor(0);
        
        FL_DrawTextBDF(4, PROJECTION_HEIGHT - 28, fps_text, 16, 640, knxt);
        FL_DrawTextBDF(4, PROJECTION_HEIGHT - 52, player_pos_text, 32, 640, knxt);
        FL_DrawTextBDF(4, PROJECTION_HEIGHT - 76, column_info, 64, 640, knxt);

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
