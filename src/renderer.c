#include "renderer.h"

#include <math.h>
#include <stdio.h>

void r_draw_column(int column, float distance, uint32_t color) {
    const float half_height = (float)FL_GetWindowHeight() / distance / 2;

    FL_DrawLine(column, (PROJECTION_HEIGHT >> 1) - half_height, column, (PROJECTION_HEIGHT >> 1) + half_height, color);
}

void r_draw_column_textured(int column, int offset, float distance, FL_Texture *texture) {
    const float height = (float)FL_GetWindowHeight() / distance;

    if(height < PROJECTION_HEIGHT) {
        const float half_height = height / 2;

        const float tex_step = GRID_SIZE / height;
        float tex_current = 0;

        uint32_t *p = texture->data + offset;
        for(int i = floor((PROJECTION_HEIGHT >> 1) - half_height); i < floor((PROJECTION_HEIGHT >> 1) + half_height); ++i) {
            uint32_t color = *(p + (int)(floor(tex_current)) * texture->width);
            uint8_t r = color >> 16, g = color >> 8, b = color;

            FL_DrawPoint(column, i, (uint32_t)(r << 16 | g << 8 | b));

            tex_current += tex_step;
        }

        //FL_DrawLine(column, 0, column, (PROJECTION_HEIGHT >> 1) - half_height - 1, 0x666666);
        //FL_DrawLine(column, (PROJECTION_HEIGHT >> 1) + half_height, column, PROJECTION_HEIGHT - 1, 0x888888);
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