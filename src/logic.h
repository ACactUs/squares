#pragma once

#include <stdlib.h>
#include <string.h>

#define SIZE_DIFF_TRESHOLD 1.2f
#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

typedef int* olist_t ;

enum no_stim    { so_idle, so_random, so_lrandom, SE_NUMBER };
enum food       { fo_idle, fo_random, fo_lrandom, fo_avoid, fo_seek, FE_NUMBER };
enum big        { bo_idle, bo_random, bo_lrandom, bo_avoid, bo_seek, BE_NUMBER };
enum pray       { po_idle, po_random, po_lrandom, po_avoid, po_seek, PE_NUMBER };

/* traits index kept in enum are used to access items in traits array */
enum traits     { ti_mspeed=0, ti_avoid_dist, ti_avoid_speed, t_pursue_speed, t_food_speed, TIE_NUMBER };

/* this type represents selected action's enum number */
typedef struct {
    enum no_stim  nostim_o;
    enum food     food_o;
    enum big      big_o;
    enum pray     pray_o;
} actions_opt_t;

typedef struct {
    int *traits;
    int size;
} traits_opt_t;

int mutate(int eo_target, int e_number, float rate);

typedef enum {
    SQ_RED,
    SQ_GREEN,
    SQ_NBLUE
} SQ_COLORS;

typedef struct {
    double y, x;
    double height, width;
    double yspeed, xspeed;
    char *name;
    SQ_COLORS color;
    size_t frags;
    int energy;
    int energy_stored;
    int ticks_idle;
    int move_time;
    actions_opt_t actions;
    traits_opt_t traits;
} rectangle_t;


rectangle_t *rectangle_create();

rectangle_t *rectangle_copy(rectangle_t *rect);


void rectangle_destroy(rectangle_t *rect);

/* modifies rectangle so that it is now dest_size times bigger than original */
void rectangle_resize_x(rectangle_t *rect, float ratio);
void rectangle_resize_y(rectangle_t *rect, float ratio);

/* returns adress of surviving rect, if both survive returns NULL 
 * this function does modify rects */
rectangle_t *rectangle_collision(rectangle_t *left, rectangle_t *right, size_t dt);

/* return adress of winning rect,
 * if none returns NULL */
rectangle_t *rectangle_compare(rectangle_t *left, rectangle_t *right);

double rectangle_size(rectangle_t *rect);



typedef struct {
    double ysize, xsize;
    size_t rect_alive;
    size_t rect_max;    /*keeps max number of rects therefore can be used by calloc*/
    rectangle_t **rects;
} plane_t;

void frame_render(plane_t plane);

void frame_simulate(plane_t plane);

void action_nostim(plane_t *plane, rectangle_t *square);
void action_food(plane_t *plane, rectangle_t *square);
void action_big(plane_t *plane, rectangle_t *square);
void action_pray(plane_t *plane, rectangle_t *square);
