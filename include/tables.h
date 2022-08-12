#ifndef __TABLES__
#define __TABLES__

#define PI 3.1415926535

extern const int ANGLE_0;
extern const int ANGLE_60;
extern const int ANGLE_30;
extern const int ANGLE_15;
extern const int ANGLE_90;
extern const int ANGLE_180;
extern const int ANGLE_270;
extern const int ANGLE_360;

extern float tan_table[3840];
extern float cos_table[3840];
extern float sin_table[3840];

void tables_init();

float arctorad(float a);

#endif