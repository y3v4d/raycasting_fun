#include "renderer.h"
#include "tables.h"

#include <math.h>
#include <stdio.h>

inline __attribute__((always_inline)) 
float absf(float n) {
    return n < 0 ? -n : n;
}

inline __attribute__((always_inline))
float apply_fish(int column, float distance) {
    //return distance;
    const float fish = (float)column / PROJECTION_WIDTH * ANGLE_60 - ANGLE_30;
    return distance * cos(arctorad(fish));
}

void r_draw_column(int column, float distance, uint32_t color) {
    const float proj_distance = (float)PROJECTION_WIDTH / 2 / tan_table[ANGLE_30];
    //const uint32_t shade = (distance == 0 ? 1 : min(255.f / absf(distance) * 2, 255));
    const float cdistance = absf(apply_fish(column, distance));
    const float half_height = (float)GRID_SIZE / cdistance * proj_distance / 2;

    FL_DrawLine(column, (PROJECTION_HEIGHT >> 1) - half_height, column, (PROJECTION_HEIGHT >> 1) + half_height, color);
}

void r_draw_column_textured(int column, int offset, float distance, FL_Texture *texture) {
    const float proj_distance = (float)PROJECTION_WIDTH / 2 / tan_table[ANGLE_30];
    //printf("proj_distance: %f\n", proj_distance);

    const float cdistance = absf(apply_fish(column, distance));
    const float height = (float)GRID_SIZE / cdistance * proj_distance;

    if(height < PROJECTION_HEIGHT) {
        const float half_height = height / 2;

        const float tex_step = GRID_SIZE / height;
        float tex_current = 0;
        float brightness = (float)GRID_SIZE / absf(cdistance);

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