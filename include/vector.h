#ifndef __VECTOR__
#define __VECTOR__

#define PI 3.1415926535

typedef struct {
    float x, y;
} vec2f_t;

typedef struct {
    int x, y;
} vec2i_t;

inline __attribute__((always_inline))
float absf(float x) {
    return x < 0 ? -x : x;
}

#endif