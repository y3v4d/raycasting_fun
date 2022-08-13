#include "minimap.h"
#include "renderer.h"

#include <follia.h>

void minimap_draw(const minimap_t *m) {
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

    float lx = m->player->x + 100 * m->player->dx;
    float ly = m->player->y - 100 * m->player->dy;

    FL_DrawCircle(m->x + m->player->x / GRID_SIZE * ratio, m->y + m->player->y / GRID_SIZE * ratio, 2, 0xFF0000, true);
    FL_DrawCircle(m->x + m->entity->x / GRID_SIZE * ratio, m->y + m->entity->y / GRID_SIZE * ratio, 2, 0x00FF00, true);

    FL_DrawLine(
        m->x + m->player->x / GRID_SIZE * ratio, 
        m->y + m->player->y / GRID_SIZE * ratio,
        m->x + lx / GRID_SIZE * ratio,
        m->y + ly / GRID_SIZE * ratio,
        0xff0000);

    FL_DrawLine(
        m->x + (lx - 50 * m->player->dy) / GRID_SIZE * ratio,
        m->y + (ly - 50 * m->player->dx) / GRID_SIZE * ratio,
        m->x + (lx + 50 * m->player->dy) / GRID_SIZE * ratio,
        m->y + (ly + 50 * m->player->dx) / GRID_SIZE * ratio,
        0x00ff00
    );

    for(int i = 0; i < m->points_count; ++i) {
        FL_DrawCircle(
            m->x + m->points[i].x / GRID_SIZE * ratio,
            m->y + m->points[i].y / GRID_SIZE * ratio,
            2,
            0x0000FF,
            true
        );
    }
}