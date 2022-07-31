#include <follia.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

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
const uint32_t map_width = 8, map_height = 8;
uint8_t map[64] = { 
    1, 3, 1, 3, 1, 3, 1, 3,
    3, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 3,
    3, 0, 0, 2, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 3,
    3, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 3,
    3, 1, 3, 1, 3, 1, 3, 1
};

vec2f_t mouse = { 0 };
char column_info[64] = "C: - D: -";
const float grid_size = 32;

void draw_column(int column, float distance, int map_x, int map_y) {
    if(column == mouse.x) {
        snprintf(column_info, 64, "C: %d D: %f", column, distance);
    }

    const uint32_t color = map[map_y * map_width + map_x] == 1 ? 0xffff00 : 0xff00ff;
    
    const float fish = (float)column / FL_GetWindowWidth() * fov - fov / 2;
    const float cdistance = distance * cos(fish * PI / 180);
    const float half_height = (float)FL_GetWindowHeight() / cdistance / 2;

    FL_DrawLine(column, (float)FL_GetWindowHeight() / 2 - half_height, column, (float)FL_GetWindowHeight() / 2 + half_height, color);
}

float tan_table[360];

int main() {
    if(!FL_Initialize(640, 480))
        exit(-1);

    FL_SetTitle("Raycasting Test");

    FL_FontBDF *knxt = FL_LoadFontBDF("data/fonts/knxt.bdf");
    if(!knxt) {
        FL_Close();
        exit(-1);
    }

    // 270 is inf 180 is 0, 90 is inf 0 is 0

    for(int i = 0; i < 360; ++i) {
        if(i == 270 || i == 90) {
            tan_table[i] = 57; 
        } else if(i == 180 || i == 0) {
            tan_table[i] = 0.001f;
        } else {
            tan_table[i] = tan((float)i * PI / 180.f);
        }
    }

   
    player_t player = { 0 };
    vec2f_t direction = { 0 };

    // search for player start position
    for(int y = 0; y < map_height; ++y) {
        for(int x = 0; x < map_width; ++x) {
            if(map[y * map_width + x] == 2) {
                map[y * map_width + x] = 0;
                player.x = x * grid_size;
                player.y = y * grid_size;
            } 
        }
    }

    FL_Timer timer = { 0 }, fps_timer = { 0 };
    char player_pos_text[32];
    char fps_text[16] = "FPS: -";

    FL_Bool first = true;

    FL_Event event;
    while(!FL_WindowShouldClose()) {
        while(FL_GetEvent(&event)) {
            if(event.type == FL_EVENT_KEY_PRESSED) {
                switch(event.key.code) {
                    case FL_KEY_d: player.vangle = 0.2f; break;
                    case FL_KEY_a: player.vangle = -0.2f; break;
                    case FL_KEY_w: player.move = 1; break;
                    case FL_KEY_s: player.move = -1; break;
                    default: break;
                }
            } else if(event.type == FL_EVENT_KEY_RELEASED) {
                switch(event.key.code) {
                    case FL_KEY_d: case FL_KEY_a: player.vangle = 0.f; break;
                    case FL_KEY_w: case FL_KEY_s: player.move = 0; break;
                    default: break;
                }
            } else if(event.type == FL_EVENT_MOUSE_MOVED) {
                mouse.x = event.mouse.x;
                mouse.y = event.mouse.y;
            }
        }

        // get delta time
        FL_StopTimer(&timer);
        const float dt = timer.delta;
        FL_StartTimer(&timer);

        // basic physics
        player.angle += player.vangle * dt;
        if(player.angle < 0) player.angle += 360.f;
        else if(player.angle > 360.f) player.angle -= 360.f;

        direction.x = cos(player.angle * PI / 180.f);
        direction.y = -sin(player.angle * PI / 180.f);

        player.x += player.move * direction.x * 0.005f * grid_size * dt;
        player.y += player.move * direction.y * -0.005f * grid_size * dt;

        snprintf(player_pos_text, 32, "X: %.2f Y: %.2f A: %.2f", player.x, player.y, player.angle);

        // setup fps on-screen text
        FL_StopTimer(&fps_timer);
        if(fps_timer.delta >= 500) {
            snprintf(fps_text, 16, "FPS: %.2f", 1000.f / timer.delta);

            FL_StartTimer(&fps_timer);
        }
      
        FL_ClearScreen();
        FL_DrawRect(0, 0, FL_GetWindowWidth(), FL_GetWindowHeight() / 2, 0x444444, true); // ceiling

        // ==== DRAW GAME ====
        const int mm_values_max = 1024;
        vec2f_t mm_values[1024] = { 0 };
        int mm_values_size = 0;

        const float r = 20;
        const int WALL_HEIGHT = 1;
        const float step = fov / FL_GetWindowWidth();

        float current = player.angle - fov / 2;
        if(current < 0) current += 360.f;

        for(int i = 0; i < FL_GetWindowWidth(); i++) {
            int v_map_x = -1, v_map_y = -1, h_map_x = -1, h_map_y = -1;
            float v_wall_distance = 999999.f, h_wall_distance = 999999.f;

            const float angle_tan = tan(current * PI / 180);

            {
                FL_Bool is_down = current >= 0 && current <= 180;

                const float Xa = grid_size / angle_tan;
                const float Ya = is_down ? grid_size : -grid_size;

                float Ay = (is_down ? floor(player.y / grid_size) * grid_size + grid_size : floor(player.y / grid_size) * grid_size - 0.0001f);
                float Ax = player.x + (player.y - Ay) / -angle_tan;

                for(int j = 0; j < r; ++j) {
                    uint8_t c = 255.f;

                    const float distance = absf(player.y - Ay) / sin(current * PI / 180);

                    int map_x = floor(Ax / grid_size), map_y = floor(Ay / grid_size);
                    if(map_x >= 0 && map_x < map_width && map_y >= 0 && map_y < map_height && map[map_y * map_width + map_x] != 0) {
                        if(mm_values_size < mm_values_max) {
                            mm_values[mm_values_size].x = map_x;
                            mm_values[mm_values_size++].y = map_y;
                        }

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
                FL_Bool is_right = current >= 270 || current <= 90;

                const float Xa = is_right ? grid_size : -grid_size;
                const float Ya = angle_tan * grid_size;

                float Ax = is_right ? floor(player.x / grid_size) * grid_size + grid_size : floor(player.x / grid_size) * grid_size - 0.0001f;
                float Ay = player.y - (Ax - player.x) * -angle_tan;

                for(int j = 0; j < r; ++j) {
                    uint8_t c = 255.f;

                    const float distance = absf(player.x - Ax) / cos(current * PI / 180);

                    int map_x = floor(Ax / grid_size), map_y = floor(Ay / grid_size);
                    if(map_x >= 0 && map_x < map_width && map_y >= 0 && map_y < map_height && map[map_y * map_width + map_x] != 0) {
                        if(mm_values_size < mm_values_max) {
                            mm_values[mm_values_size].x = Ax;
                            mm_values[mm_values_size++].y = Ay;
                        }

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
                draw_column(i, h_wall_distance, h_map_x, h_map_y);
            } else if(absf(v_wall_distance) < absf(h_wall_distance) && absf(v_wall_distance) < r) {
                draw_column(i, v_wall_distance, v_map_x, v_map_y);
            }

            current += step;
            if(current > 360.f) current -= 360.f;
        }

        // ==== DRAW MAP ====
        const int mm_w = 128, mm_h = 128;
        const int mm_x = FL_GetWindowWidth() - mm_w - 8, mm_y = 8;
        const float mm_ratio = (float)mm_w / map_width;

        // background
        FL_DrawRect(mm_x, mm_y, mm_w, mm_h, 0x4444AA, true);
        FL_DrawRect(mm_x, mm_y, mm_w, mm_h, 0, false);

        for(int y = 0; y < map_height; ++y) {
            for(int x = 0; x < map_width; ++x) {
                int sx = mm_x + mm_ratio * x, sy = mm_y + mm_ratio * y;

                int tile = map[y * map_width + x];
                if(tile == 1) {
                    FL_DrawRect(sx, sy, mm_ratio, mm_ratio, 0xFFFF00, true);
                } else if(tile == 3) {
                    FL_DrawRect(sx, sy, mm_ratio, mm_ratio, 0xFF00FF, true);
                }

                FL_DrawRect(mm_x + mm_ratio * x, mm_y + mm_ratio * y, mm_ratio, mm_ratio, 0, false);
            }
        }

        for(int i = 0; i < mm_values_size; ++i) {
            vec2f_t *v = &mm_values[i];
            FL_DrawCircle(mm_x + v->x * mm_ratio, mm_y + v->y * mm_ratio, 1, 0xffffff, true);
        }

        // player
        FL_DrawCircle(mm_x + player.x / grid_size * mm_ratio, mm_y + player.y / grid_size * mm_ratio, 2, 0xFF0000, true);

        FL_DrawCircle(mouse.x, mouse.y, 2, 0x0000ff, true);
        FL_DrawLine(mouse.x, 0, mouse.x, FL_GetWindowHeight(), 0x0000ff);

        FL_SetTextColor(0);
        
        FL_DrawTextBDF(4, FL_GetWindowHeight() - 28, fps_text, 16, 640, knxt);
        FL_DrawTextBDF(4, FL_GetWindowHeight() - 52, player_pos_text, 32, 640, knxt);
        FL_DrawTextBDF(4, FL_GetWindowHeight() - 76, column_info, 64, 640, knxt);

        FL_Render();
    }

    FL_FreeFontBDF(knxt);
    FL_Close();

    return 0;
}
