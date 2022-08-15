// === DRAW ENTITIES ===
{
    vec2f_t Z = {
        .x = entity.x - player.x,
        .y = entity.y - player.y
    };

    const float w = 1.f / -(player.px * player.dy - player.py * player.dx);

    float rx = w * (Z.x * player.dy - Z.y * player.dx);
    float t = w * (Z.x * player.py - Z.y * player.px);

    float x = (PROJECTION_WIDTH >> 1) * rx / t;

    float half_screen = PROJECTION_HEIGHT >> 1;
    float size = (float)PROJECTION_HEIGHT / t;

    if(t > 0) {
        for(int i = floorf(x - size / 2); i < floorf(x + size / 2); ++i) {
            int col = (PROJECTION_WIDTH >> 1) - i;
            if(col < 0 || col >= PROJECTION_WIDTH) continue;

            if(t < z_buffer[col]) {
                //FL_DrawLine((PROJECTION_WIDTH >> 1) - i, half_screen - size / 2, (PROJECTION_WIDTH >> 1) - i, half_screen + size / 2, 0xffff00);
            }
        }
    }
}