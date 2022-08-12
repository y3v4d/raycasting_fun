#include "tables.h"
#include "renderer.h"

#include <math.h>

const int ANGLE_0 = 0;
const int ANGLE_60 = PROJECTION_WIDTH;
const int ANGLE_30 = ANGLE_60 / 2;
const int ANGLE_15 = ANGLE_30 / 2;
const int ANGLE_90 = ANGLE_30 * 3;
const int ANGLE_180 = ANGLE_60 * 3;
const int ANGLE_270 = ANGLE_90 * 3;
const int ANGLE_360 = ANGLE_180 * 2;

float tan_table[3840]; // ANGLE_360 can't seem to work without warnings...
float cos_table[3840];
float sin_table[3840];

void tables_init() {
    for(int i = 0; i < ANGLE_360; ++i) {
        tan_table[i] = tan(arctorad(i));
        sin_table[i] = sin(arctorad(i));
        cos_table[i] = cos(arctorad(i));
    }
}

float arctorad(float a) {
    return a * PI / ANGLE_180;
}