#include "renderer.h"

#include <stdlib.h>

#include <math.h>
#include <stdio.h>

float z_buffer[PROJECTION_WIDTH] = { 0 };

float test = 0;

inline __attribute__((always_inline))
float absf(float n) {
    return n < 0 ? -n : n;
}

void r_draw_column(int column, float distance, uint32_t color) {
    const float half_height = (float)PROJECTION_HEIGHT / distance / 2;

    FL_DrawLine(column, (PROJECTION_HEIGHT >> 1) - half_height, column, (PROJECTION_HEIGHT >> 1) + half_height, color);
}

void r_draw_column_textured_alpha(int column, float offset, float distance, FL_Texture *texture) {
    uint32_t *p = FL_GetFrameBuffer();
    const float height = (float)PROJECTION_HEIGHT / distance;

    int draw_end = (PROJECTION_HEIGHT >> 1) + height / 2;
    int draw_start = draw_end - height;

    if(draw_start < 0) draw_start = 0;
    if(draw_end >= PROJECTION_HEIGHT) draw_end = PROJECTION_HEIGHT - 1;

    const int tex_x = (int)floorf(offset * texture->width) & (texture->width - 1);
    const float tex_step = (float)texture->height / height;
    float tex_pos = draw_start > 0 ? 0 : ((height - PROJECTION_HEIGHT) / 2) * tex_step;

    for(int i = draw_start; i < draw_end; ++i) {
        int tex_y = (int)tex_pos & (texture->height - 1);
        tex_pos += tex_step;
        
        uint32_t color = texture->data[tex_y * texture->width + tex_x];
        if(color == 0) continue;

        *(p + column + i * PROJECTION_WIDTH) = color;
    }
}

void draw_column_textured(int column, float offset, float distance, float wall_height, float wall_z, FL_Texture *texture) {
    uint32_t *p = FL_GetFrameBuffer();

    const float player_z = PROJECTION_HEIGHT >> 1;
    const float z = player_z - wall_z;

    const float height = (float)wall_height / distance;

    int draw_end = (PROJECTION_HEIGHT >> 1) + z / distance;
    int draw_start = draw_end - height;

    if(draw_start < 0) draw_start = 0;
    if(draw_end >= PROJECTION_HEIGHT) draw_end = PROJECTION_HEIGHT - 1;
    if(draw_end >= test) draw_end = test;

    const int tex_x = (int)floorf(offset * texture->width) & (texture->width - 1);
    const float tex_step = (float)texture->height / height;
    float tex_pos = draw_start > 0 ? 0 : ((height - PROJECTION_HEIGHT) / 2) * tex_step;

    for(int i = draw_start; i < draw_end; ++i) {
        int tex_y = (int)tex_pos & (texture->height - 1);
        
        uint32_t color = texture->data[tex_y * texture->width + tex_x];
        tex_pos += tex_step;

        *(p + column + i * PROJECTION_WIDTH) = color;
    }

    test = draw_start;
}

void r_draw_column_textured(int column, float offset, float distance, FL_Texture *texture) {
    uint32_t *p = FL_GetFrameBuffer();

    const float wall_height = PROJECTION_HEIGHT >> 1;
    const float player_z = PROJECTION_HEIGHT >> 1;
    const float wall_z = 0;

    const float z = player_z - wall_z;

    const float height = (float)wall_height / distance;

    int draw_end = (PROJECTION_HEIGHT >> 1) + z / distance;
    int draw_start = draw_end - height;

    if(draw_start < 0) draw_start = 0;
    if(draw_end >= PROJECTION_HEIGHT) draw_end = PROJECTION_HEIGHT - 1;

    const int tex_x = (int)floorf(offset * texture->width) & (texture->width - 1);
    const float tex_step = (float)texture->height / height;
    float tex_pos = draw_start > 0 ? 0 : ((height - PROJECTION_HEIGHT) / 2) * tex_step;

    for(int i = draw_start; i < draw_end; ++i) {
        int tex_y = (int)tex_pos & (texture->height - 1);
        
        uint32_t color = texture->data[tex_y * texture->width + tex_x];
        tex_pos += tex_step;

        *(p + column + i * PROJECTION_WIDTH) = color;
        //FL_DrawPoint(column, i, color);
    }
}

void r_draw_walls(const map_t *map, const player_t *p) {
    for(int i = 0; i < PROJECTION_WIDTH; ++i) {
        test = PROJECTION_HEIGHT;
        int mx = floorf(p->x), my = floorf(p->y); // map coordinates of the player
        
        float cam_x = (float)i * 2 / PROJECTION_WIDTH - 1;

        float ray_dx = p->dx + p->px * cam_x;
        float ray_dy = p->dy + p->py * cam_x;

        // only relative distance matter, not the exact one
        float delta_x = absf(1.f / ray_dx);
        float delta_y = absf(1.f / ray_dy);

        float curr_dx, curr_dy;

        int map_step_x = 0, map_step_y = 0;
        if(ray_dx >= 0) {
            map_step_x = 1;
            curr_dx = (mx + 1 - p->x) * delta_x;
        } else {
            map_step_x = -1;
            curr_dx = (p->x - mx) * delta_x;
        }

        if(ray_dy >= 0) {
            map_step_y = 1;
            curr_dy = (my + 1 - p->y) * delta_y;
        } else {
            map_step_y = -1;
            curr_dy = (p->y - my) * delta_y;
        }

        int side = 0; // 0 - horizontal 1 - vertical
        int hit = 0;
        for(int j = 0; j < 50; ++j) {
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

                float distance = (side == 0 ? curr_dy - delta_y : curr_dx - delta_x);
                float offset;
                if(side == 0) {
                    offset = p->x + distance * ray_dx;
                    offset -= floorf(offset);
                } else {
                    offset = p->y + distance * ray_dy;
                    offset -= floorf(offset);
                }

                float height = 0;
                if(hit == 1) height = PROJECTION_HEIGHT;
                else height = 150;

                draw_column_textured(i, offset, distance, height, 0, wall0);
                z_buffer[i] = distance;
            }
        }        
    }
}

void r_draw_floor(const map_t *map, const player_t *p) {
    uint32_t *fb = FL_GetFrameBuffer();
    float ray_dir0_x = p->dx - p->px;
    float ray_dir0_y = p->dy - p->py;
    float ray_dir1_x = p->dx + p->px;
    float ray_dir1_y = p->dy + p->py;

    for(int i = PROJECTION_HEIGHT >> 1; i < PROJECTION_HEIGHT; ++i) {
        int d = (PROJECTION_HEIGHT >> 1) - i;

        const float player_z = PROJECTION_HEIGHT >> 1;

        float r = -player_z / d;

        float floorStepX = r * (ray_dir1_x - ray_dir0_x) / PROJECTION_WIDTH;
        float floorStepY = r * (ray_dir1_y - ray_dir0_y) / PROJECTION_WIDTH;

        float floorX = p->x + ray_dir0_x * r;
        float floorY = p->y + ray_dir0_y * r;

        uint32_t *t = floor0->data;
        uint32_t *fbc = fb + i * PROJECTION_WIDTH;

        int x = PROJECTION_WIDTH;
        while(x--) {
            int mx = floorf(floorX), my = floorf(floorY);

            int tx = (int)(floor0->width * (floorX - mx)) & (floor0->width - 1);
            int ty = (int)(floor0->height * (floorY - my)) & (floor0->height - 1);

            floorX += floorStepX;
            floorY += floorStepY;


            uint32_t color = *(t + ty * floor0->width + tx);
            *(fbc + PROJECTION_WIDTH - 1 - x) = color;
            //*(fb + (PROJECTION_HEIGHT - i - 1) * PROJECTION_WIDTH + PROJECTION_WIDTH - 1 - x) = color;
        }
    }
}
