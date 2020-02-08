#include "logic.h"
#include <math.h>

rectangle_t *rectangle_create() {
    rectangle_t *rect;
    rect = (rectangle_t*)calloc(1, sizeof(rectangle_t));
    return rect;
}

void rectangle_destroy(rectangle_t *rect) {
    free(rect);
}


void rectangle_resize_x(rectangle_t *rect, float ratio) { 
    if ( !ratio ) return;
    rect->width *= ratio;
}

void rectangle_resize_y(rectangle_t *rect, float ratio) { 
    if ( !ratio ) return;
    rect->height *= ratio;
}

rectangle_t *rectangle_copy(rectangle_t *rect) {
    rectangle_t *copy = rectangle_create();
    memcpy(copy, rect, sizeof(rectangle_t));
    return copy;
}

rectangle_t *rectangle_collision(rectangle_t *left, rectangle_t *right, size_t dt) {
    if ( left == right ) return NULL; 
    rectangle_t *winner = rectangle_compare(left, right);

    if ( !winner ) {
        if ( !(rand() % 10)) 
            return NULL;

        if ( rand() % 2 ) {
            rectangle_resize_x(left, 0.9f);
            rectangle_resize_y(right, 0.9f);
        } else {
            rectangle_resize_y(left, 0.9f);
            rectangle_resize_x(right, 0.9f);
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
    double  lsize, rsize;
    lsize = rectangle_size(left),
    rsize = rectangle_size(right);
    double delta = MAX(lsize, rsize) / MIN(rsize, lsize); 
    if ( delta < SIZE_DIFF_TRESHOLD ) return NULL;
    return MAX(rsize, lsize) == rsize ? right : left;
}

double rectangle_size(rectangle_t *rect) { 
    return (double)(rect->width * rect->height);
}


/*FIXME add rects count variable and remove it from plane_create*/
void  plane_init(plane_t *plane, rectangle_t **rects, size_t recs_size) {
    plane->rects = calloc(sizeof(rectangle_t*), recs_size);
    plane->rect_max = recs_size;
    if (!rects){
        /*no rects given, therefore create them accordingly to config globals*/
        size_t i;
        for(i = 0; i < recs_size; i++) {
            /*FIXME FIXME FIXME hardcoding rules*/
            rectangle_t *rect = plane->rects[i] = rectangle_create();
            rect->width     = 20;
            rect->height    = 7;
            rect->x         = 40;
            rect->y         = 4;
        }
    } else { 
        /*load existing rects*/
        exit(1); 
    }
}

plane_t *plane_create(double xsize, double ysize) {
    plane_t *plane = calloc(sizeof(plane_t), 1);
    plane->xsize = xsize;
    plane->ysize = ysize;
    return plane;
}

void plane_destroy(plane_t *plane) {
    int i, max;
    max = plane->rect_max;
    rectangle_t **rects = plane->rects;
    for(i = 0; i < max; i++) {
        if (!rects[i]) continue;
        free(rects[i]);
    }
    free(rects);
    free(plane);
}
