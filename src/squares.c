#include "squares.h"
#include <math.h>

rectangle_t *rectangle_create() {
    rectangle_t *rect;
    rect = (rectangle_t*)calloc(1, sizeof(rectangle_t));
    return rect;
}

void rectangle_destroy(rectangle_t *rect) {
    free(rect);
}


void rectangle_resize_x(rectangle_t *rect, float dest_size) { 
    float delta = (rect->x2 - rect->x1)*dest_size - (rect->x2 - rect->x1);
    rect->x2 = rect->x2 + floor(delta/2);
    rect->x1 = rect->x1 + ceil(delta/2);
}

void rectangle_resize_y(rectangle_t *rect, float dest_size) { 
    float delta = (rect->x2 - rect->x1)*dest_size - (rect->x2 - rect->x1);
    rect->x2 = rect->x2 + floor(delta/2);
    rect->x1 = rect->x1 + ceil(delta/2);
}




rectangle_t *rectangle_copy(rectangle_t *rect) {
    rectangle_t *copy = rectangle_create();
    memcpy(copy, rect, sizeof(rectangle_t));
    return copy;
}

rectangle_t *rectangle_collision(rectangle_t *left, rectangle_t *right, size_t dt) {
    //rectangle_t *rect   = (rectangle_t*) calloc(2, sizeof(rectangle_t));
    rectangle_t *winner = rectangle_compare(left, right);

    if ( !winner ) {
        if ( !(rand() % 10)) 
            return NULL;

        if ( rand() % 2) {
            rectangle_resize_x(left, 0.9);
            rectangle_resize_y(right, 0.9);
        } else {
            rectangle_resize_y(left, 0.9);
            rectangle_resize_x(right, 0.9);
        }
    }
    
    //TODO
    /* some genetic logic may be defined here
     * for now just kill smaller one */
    rectangle_t *looser = (winner == left) ? right : left;

    rectangle_destroy(looser);

    return winner;
}

rectangle_t *rectangle_compare(rectangle_t *left, rectangle_t *right) { 
    size_t  lsize = rectangle_size(left),
            rsize = rectangle_size(right);
    size_t delta = MAX(lsize, rsize) / MIN(rsize, lsize); if ( delta < SIZE_DIFF_TRESHOLD ) return NULL;
    return MAX(rsize, lsize) == rsize ? right : left;
}

size_t rectangle_size(rectangle_t *rect) { 
    size_t length = rect->x2 - rect->x1;
    size_t height = rect->y2 - rect->y1;
    return length * height;
}
