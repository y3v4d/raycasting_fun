#include "renderer.h"

#include <math.h>
#include <stdio.h>

float z_buffer[PROJECTION_WIDTH] = { 0 };

inline __attribute__((always_inline))
float absf(float n) {
    return n < 0 ? -n : n;
}

void r_draw_column(int column, float distance, uint32_t color) {
    const float half_height = (float)PROJECTION_HEIGHT / distance / 2;

    FL_DrawLine(column, (PROJECTION_HEIGHT >> 1) - half_height, column, (PROJECTION_HEIGHT >> 1) + half_height, color);
}

void r_draw_column_textured(int column, float offset, float distance, FL_Texture *texture) {
    const float height = (float)FL_GetWindowHeight() / distance;

    if(height < PROJECTION_HEIGHT) {
        const float half_height = height / 2;

        const float tex_step = texture->height / height;
        float tex_current = 0;

        uint32_t *p = texture->data + (int)floorf(offset * texture->width);
        for(int i = floorf((PROJECTION_HEIGHT >> 1) - half_height); i < floorf((PROJECTION_HEIGHT >> 1) + half_height); ++i) {
            uint32_t color = *(p + (int)(floorf(tex_current)) * texture->width);
            FL_DrawPoint(column, i, color);

            tex_current += tex_step;
        }
    } else {
        const float tex_step = texture->height / height;
        
        float diff = height - PROJECTION_HEIGHT;
        float tex_current = diff / 2 * tex_step;

        uint32_t *p = texture->data + (int)floorf(offset * texture->width);
        for(int i = 0; i < PROJECTION_HEIGHT; ++i) {
            FL_DrawPoint(column, i, *(p + (int)(floorf(tex_current) * texture->width)));

            tex_current += tex_step;
        }
    }
}

void r_draw_walls(const map_t *map, const player_t *p) {
    for(int i = 0; i < PROJECTION_WIDTH; ++i) {
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
            offset = p->x + distance * ray_dx;
            offset -= floorf(offset);
        } else {
            offset = p->y + distance * ray_dy;
            offset -= floorf(offset);
        }

        if(hit) {
            float lineHeight = (float)PROJECTION_HEIGHT / distance;

            r_draw_column_textured(i, offset, distance, wall0);
            z_buffer[i] = distance;
        } else {
            z_buffer[i] = 1e30;
        }
    }
}

void r_draw_floor(const player_t *p) {
    for(int i = PROJECTION_HEIGHT >> 1; i < PROJECTION_HEIGHT; ++i) {
        int d = (PROJECTION_HEIGHT >> 1) - i;

        float ray_dir0_x = p->dx - p->px;
        float ray_dir0_y = p->dy - p->py;
        float ray_dir1_x = p->dx + p->px;
        float ray_dir1_y = p->dy + p->py;

        const float player_z = PROJECTION_HEIGHT >> 1;

        float r = -player_z / d;

        float floorStepX = r * (ray_dir1_x - ray_dir0_x) / PROJECTION_WIDTH;
        float floorStepY = r * (ray_dir1_y - ray_dir0_y) / PROJECTION_WIDTH;

        float floorX = p->x + ray_dir0_x * r;
        float floorY = p->y + ray_dir0_y * r;

        for(int x = 0; x < PROJECTION_WIDTH; ++x) {
            int mx = floorf(floorX), my = floorf(floorY);

            int tx = (int)(floor0->width * (floorX - mx)) & (floor0->width - 1);
            int ty = (int)(floor0->height * (floorY - my)) & (floor0->height - 1);

            floorX += floorStepX;
            floorY += floorStepY;

            FL_DrawPoint(x, i, floor0->data[ty * floor0->width + tx]);
        }
    }
}