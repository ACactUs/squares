#include <stdlib.h>
#include "squares.h"

typedef struct {
    size_t ysize, xsize;
    size_t rect_count;
    rectangle_t **rect_array;
    // Decorations
} plane_t;

void frame_render(plane_t plane);

void frame_simulate(plane_t plane_);
