#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logic.h"

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

const float SIZE_DIFF_TRESHOLD = 1.2;


typedef enum {
    RED,
    GREEN,
    BLUE
} COLORS;

typedef struct {
    int y1, x1, y2, x2;
    double yspeed, xspeed;
    char *name;
    COLORS color;
    size_t frags;
} rectangle_t;


rectangle_t *rectangle_create();

rectangle_t *rectangle_copy(rectangle_t *rect);


void rectangle_destroy(rectangle_t *rect);

/* modifies rectangle so that it is now dest_size times bigger than original */
void rectangle_resize_x(rectangle_t *rect, float dest_size);
void rectangle_resize_y(rectangle_t *rect, float dest_size);

/* returns adress of surviving rect, if both survive returns NULL 
 * this function does modify rects */
rectangle_t *rectangle_collision(rectangle_t *left, rectangle_t *right, size_t dt);

/* return adress of winning rect,
 * if none returns NULL */
rectangle_t *rectangle_compare(rectangle_t *left, rectangle_t *right);

size_t rectangle_size(rectangle_t *rect);
