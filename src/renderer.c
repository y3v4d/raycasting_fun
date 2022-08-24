#include "renderer.h"

#include <stdlib.h>

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
    uint32_t *fb = FL_GetFrameBuffer();

    for(int i = 0; i < PROJECTION_WIDTH; ++i) {
        int mx = floorf(p->x), my = floorf(p->y); // map coordinates of the player
        if(my < 0 || my >= map->height || mx < 0 || mx >= map->width) break;
        
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
        int hit = map->data[my * map->width + mx];

        float highest_top = PROJECTION_HEIGHT - 1;
        float last_wall_height = 0;
        if(hit == 1) last_wall_height = PROJECTION_HEIGHT;
        else if(hit == 2) last_wall_height = PROJECTION_HEIGHT >> 2;

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

            const float distance = (side == 0 ? curr_dy - delta_y : curr_dx - delta_x);

            // first try to draw the topside of the wall
            float x = (PROJECTION_HEIGHT >> 1) + (p->z - last_wall_height) / distance;
            if(x < highest_top) {
                if(x < 0) x = 0;
                for(int y = x; y < highest_top; ++y) {
                    //if(last_wall_height == 0) break;
                    uint32_t color = last_wall_height == 0 ? 0xff0000 : 0x0000ff;
                    *(fb + i + y * PROJECTION_WIDTH) = color;
                }

                highest_top = x;
            }

            // return if out of bounds
            if(my < 0 || my >= map->height || mx < 0 || mx >= map->width) break;

            hit = map->data[my * map->width + mx]; // what cell was hit

            float wall_height = 0;
            if(hit == 1) wall_height = PROJECTION_HEIGHT;
            else if(hit == 2) wall_height = PROJECTION_HEIGHT >> 2;
            last_wall_height = wall_height;

            // draw wall if there is one
            if(wall_height != 0) {
                const float height = (float)wall_height / distance;

                float wall_bottom = (PROJECTION_HEIGHT >> 1) + p->z / distance;
                float wall_top = wall_bottom - height;

                if(wall_top < 0) wall_top = 0;
                if(wall_top >= highest_top) continue;
                if(wall_bottom >= PROJECTION_HEIGHT) wall_bottom = PROJECTION_HEIGHT - 1;
                if(wall_bottom > highest_top) wall_bottom = highest_top;

                for(int y = wall_top; y < wall_bottom; ++y) {
                    uint32_t color = side == 0 ? 0xffff00 : 0xaaaa00;
                    *(fb + i + y * PROJECTION_WIDTH) = color;
                }

                if(wall_top < highest_top) highest_top = wall_top;
            }

            /*
                float offset;
                if(side == 0) {
                    offset = p->x + distance * ray_dx;
                    offset -= floorf(offset);
                } else {
                    offset = p->y + distance * ray_dy;
                    offset -= floorf(offset);
                }
            }*/

            
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

        float r = -p->z / d;

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

            if(mx < 0 || mx >= map->width || my < 0 || my >= map->height) continue;
            uint8_t *tile = map->data + my * map->width + mx;

            uint32_t color = *(t + ty * floor0->width + tx);
            *(fbc + PROJECTION_WIDTH - 1 - x) = color;
            //*(fb + (PROJECTION_HEIGHT - i - 1) * PROJECTION_WIDTH + PROJECTION_WIDTH - 1 - x) = color;
        }
    }
}
