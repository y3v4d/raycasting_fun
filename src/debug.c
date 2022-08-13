#include "debug.h"

#include <follia.h>
#include <stdio.h>

int averages_counter = 0;
double averages[FL_TIMER_ALL + 1][STATS_AVERAGE_COUNT];

double average_array(double *a, int p) {
    double total = 0;
    for(int i = 0; i < p; ++i) {
        total += a[i];
    }

    return total / p;
}

void debug_update_stats() {
    averages[FL_TIMER_CLEAR_SCREEN][averages_counter] = FL_GetCoreTimer(FL_TIMER_CLEAR_SCREEN);
    averages[FL_TIMER_RENDER][averages_counter] = FL_GetCoreTimer(FL_TIMER_RENDER);
    averages[FL_TIMER_CLEAR_TO_RENDER][averages_counter] = FL_GetCoreTimer(FL_TIMER_CLEAR_TO_RENDER);
    averages[FL_TIMER_ALL][averages_counter] = FL_GetCoreTimer(FL_TIMER_ALL);

    ++averages_counter;
    if(averages_counter >= STATS_AVERAGE_COUNT) averages_counter = 0;
}

void debug_print_stats(char *s, int max_size) {
    const double dt = FL_GetDeltaTime();

    snprintf(
        s, max_size, 
        "Clear: %f\nRender: %f\nClear to render: %f\nAll: %f\nDelta: %f\nFPS: %f",
        average_array(averages[FL_TIMER_CLEAR_SCREEN], STATS_AVERAGE_COUNT),
        average_array(averages[FL_TIMER_RENDER], STATS_AVERAGE_COUNT),
        average_array(averages[FL_TIMER_CLEAR_TO_RENDER], STATS_AVERAGE_COUNT),
        average_array(averages[FL_TIMER_ALL], STATS_AVERAGE_COUNT),
        dt,
        1000.0 / dt
    );
}