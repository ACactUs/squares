#include "logic.h"
#include <math.h>

rectangle_t *rectangle_create() {
    rectangle_t *rect; rect = (rectangle_t*)calloc(1, sizeof(rectangle_t));
    return rect;
}

void rectangle_destroy(rectangle_t *rect) {
    free(rect);
}


void rectangle_resize_x(rectangle_t *rect, float ratio) { 
    //if ( !ratio ) return;
    rect->width *= ratio;
}

void rectangle_resize_y(rectangle_t *rect, float ratio) { 
    //if ( !ratio ) return;
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

void action_nostim(plane_t *plane, size_t index) {
    rectangle_t *rect = plane->rects[index];
    if (!rect) return;
    switch (rect->actions.nostim_o) {
        case so_idle:
            rectangle_hybernate(plane, index);
            break;
        case so_random:
            rectangle_move_random(plane, index);
            break;
        case so_lrandom:
            rectangle_move_lrandom(plane, index);
            break;
        case SE_NUMBER:
            exit(1);
    }
}


void action_food(plane_t *plane, size_t index) {
    rectangle_t *rect = plane->rects[index];
    if (!rect) return;
    switch (rect->actions.food_o) {
        case fo_idle:
            rectangle_hybernate(plane, index);
            break;
        case fo_random:
            rectangle_move_random(plane, index);
            break;
        case fo_lrandom:
            rectangle_move_lrandom(plane, index);
            break;
        case fo_avoid:
            rectangle_move_avoid(plane, index);
            break;
        case fo_seek:
            rectangle_move_seek(plane, index);
            break;
        case FE_NUMBER:
            exit(1);
    }
}

void action_big(plane_t *plane, size_t index) {
    rectangle_t *rect = plane->rects[index];
    if (!rect) return;
    switch (rect->actions.food_o) {
        case bo_idle:
            rectangle_hybernate(plane, index);
            break;
        case bo_random:
            rectangle_move_random(plane, index);
            break;
        case bo_lrandom:
            rectangle_move_lrandom(plane, index);
            break;
        case bo_avoid:
            rectangle_move_avoid(plane, index);
            break;
        case bo_seek:
            rectangle_move_seek(plane, index);
            break;
        case BE_NUMBER:
            exit(1);
    }
}

void action_pray(plane_t *plane, size_t index) {
    rectangle_t *rect = plane->rects[index];
    if (!rect) return;
    switch (rect->actions.food_o) {
        case po_idle:
            rectangle_hybernate(plane, index);
            break;
        case po_random:
            rectangle_move_random(plane, index);
            break;
        case po_lrandom:
            rectangle_move_lrandom(plane, index);
            break;
        case po_avoid:
            rectangle_move_avoid(plane, index);
            break;
        case po_seek:
            rectangle_move_seek(plane, index);
            break;
        case PE_NUMBER:
            exit (1);
    }
}

void rectangle_hybernate    (plane_t *plane, size_t index) {exit(1);}
void rectangle_move_random  (plane_t *plane, size_t index) {exit(1);}
void rectangle_move_lrandom (plane_t *plane, size_t index) {exit(1);}
void rectangle_move_avoid   (plane_t *plane, size_t index) {exit(1);}
void rectangle_move_seek    (plane_t *plane, size_t index) {exit(1);}

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

void _rectangle_init_randomly(plane_t *plane, rectangle_t *rect) {
    rect->width     = rand() % (WIDTH_INIT_MAX - WIDTH_INIT_MIN + 1) + WIDTH_INIT_MIN;
    rect->height    = rand() % (HEIGHT_INIT_MAX - HEIGHT_INIT_MIN + 1) + HEIGHT_INIT_MIN;
    rect->x         = rand() % (int)(plane->xsize - rect->width);
    rect->y         = rand() % (int)(plane->ysize - rect->height);
    rect->energy    = 100; //FIXME has no meaning
    //TODO initialize actions randomly
}

/*FIXME add rects count variable and remove it from plane_create*/
void  plane_init(plane_t *plane, rectangle_t **rects, size_t recs_size) {
    plane->rects = calloc(sizeof(rectangle_t*), recs_size);
    plane->rect_max = recs_size;
    if (!rects){
        /* no rects given, therefore create them accordingly to config globals
         * TODO kill intersecting ones*/
        size_t i;
        for (i = 0; i < recs_size; i++) {
            rectangle_t *rect = plane->rects[i] = rectangle_create();
            _rectangle_init_randomly(plane, rect);
            size_t j = 0;
            while (j++ < RECTANGLE_INIT_MAX_RETRIES) 
                if (!plane_check_collisions(plane, i)) continue;
                _rectangle_init_randomly(plane, rect);        
        }
        /* TODO beter initialization*/
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
    size_t i, max;
    max = plane->rect_max;
    rectangle_t **rects = plane->rects;
    for(i = 0; i < max; i++) {
        if (!rects[i]) continue;
        free(rects[i]);
    }
    free(rects);
    free(plane);
}


/* Checks for collisions for rectangle of given index 
 * WARN assumes that index is valid 
 * this function does check for NULL rectangles at index */
rectangle_t *plane_check_collisions(plane_t *plane, size_t index) {
    size_t i;
    rectangle_t *rect = plane->rects[index];
    if (!rect) return NULL;
    for (i = 0; i < plane->rect_max; i++) {
        rectangle_t *curr = plane->rects[i];
        if (!curr) continue;
        if (rectangle_check_collision(curr, rect) && rect != curr) return curr;
    }
    return NULL;
}

/*WARN function asumes that left and right exist and were initialized correctly*/
int rectangle_check_collision(rectangle_t *left, rectangle_t *right) {
    if (right->x > left->x && right->x < left->x + left->width){
        if (right->y > left->y && right->y < left->y + left->width)
            return 1;
        if (right->y + right->width > left->y && right->y + right->width < left->y + left->width)
            return 1;
    }
    if (right->x + right->width > left->x && right->x + right->width < left->x + left->width) {
        if(right->y > left->y && right->y < left->y + left->width)
            return 1;
        if(right->y + right->width > left->y && right->y + right->width < left->y + left->width)
            return 1;
    }
    return 0;
}
