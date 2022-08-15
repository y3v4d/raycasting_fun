#include "minimap.h"
#include "renderer.h"

#include <follia.h>

void minimap_draw(const minimap_t *m) {
    FL_DrawRect(m->x, m->y, m->w, m->h, 0x000066, true);
    
    const float ratio = (float)m->w / m->map->width;

    for(int y = 0; y < m->map->height; ++y) {
        for(int x = 0; x < m->map->width; ++x) {
            int sx = m->x + ratio * x, sy = m->y + ratio * y;

            int tile = m->map->data[y * m->map->width + x];
            if(tile != 0) {
                FL_DrawRect(sx, sy, ratio, ratio, 0xFFFF00, true);
                FL_DrawRect(m->x + ratio * x, m->y + ratio * y, ratio, ratio, 0, false);
            }
        }
    }

    float lx = m->player->x + 2 * m->player->dx;
    float ly = m->player->y + 2 * m->player->dy;

    FL_DrawCircle(m->x + m->player->x * ratio, m->y + m->player->y * ratio, 2, 0xFF0000, true);
    if(m->entity) FL_DrawCircle(m->x + m->entity->x * ratio, m->y + m->entity->y * ratio, 2, 0x00FF00, true);

    FL_DrawLine(
        m->x + m->player->x * ratio, 
        m->y + m->player->y * ratio,
        m->x + lx * ratio,
        m->y + ly * ratio,
        0xff0000);

    for(int i = 0; i < m->points_count; ++i) {
        FL_DrawCircle(
            m->x + m->points[i].x * ratio,
            m->y + m->points[i].y * ratio,
            2,
            0x0000FF,
            true
        );
    }
}