#include "renderer.h"

#include <math.h>
#include <stdio.h>

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