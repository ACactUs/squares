#pragma once

#include <stdlib.h>
#include <string.h>
#include <time.h>

/*CONFIG*/
#define SIZE_DIFF_TRESHOLD          1.2f
#define WIDTH_INIT_MIN              4
#define WIDTH_INIT_MAX              10
#define HEIGHT_INIT_MIN             4
#define HEIGHT_INIT_MAX             10
#define SPEED_INIT_MAX              4
#define SPEED_ABS_MAX               15
#define RECTANGLE_INIT_MAX_RETRIES  10
#define TICK_NSEC                   5000000LL   /* 500ticks/sec */
#define BOUNCE_SPEED_FINE           0.4         /* spd = spd - spd*fine*/
/*END CONFIG*/

/* IMPORTANT NOTICE
 * speed is given in distance units per second */

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

typedef int* olist_t ;

enum no_stim    { so_idle, so_random, so_lrandom, SE_NUMBER };
enum food       { fo_idle, fo_random, fo_lrandom, fo_avoid, fo_seek, FE_NUMBER };
enum big        { bo_idle, bo_random, bo_lrandom, bo_avoid, bo_seek, BE_NUMBER };
enum prey       { po_idle, po_random, po_lrandom, po_avoid, po_seek, PE_NUMBER };

enum actions    { a_no_stim, a_food, a_big, a_prey }; 

/* traits index kept in enum are used to access items in traits array */
enum traits     { ti_mspeed=0, ti_avoid_dist, ti_avoid_speed, t_pursue_speed, t_food_speed, TIE_NUMBER };

/* this type represents selected action's enum number */
typedef struct {
    enum no_stim  nostim_o;
    enum food     food_o;
    enum big      big_o;
    enum prey     prey_o;
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
    double energy;
    double energy_stored;
    size_t ticks_idle;          /* number of ticks w/o stimuli */
    struct timespec timer;      /* timespec of an event */
    int move_time;              /* ??? */
    actions_opt_t actions;
    traits_opt_t traits;
} rectangle_t;


rectangle_t *rectangle_create(); /*done*/

rectangle_t *rectangle_copy(rectangle_t *rect); /*done*/



void rectangle_destroy(rectangle_t *rect); /*done*/

int rectangle_check_collision(rectangle_t *left, rectangle_t *right); /*done*/

/* modifies rectangle so that it is now dest_size times bigger than original */
void rectangle_resize_x(rectangle_t *rect, float ratio); /*done*/
void rectangle_resize_y(rectangle_t *rect, float ratio); /*done*/


/* return adress of winning rect,
 * if none returns NULL */
rectangle_t *rectangle_compare(rectangle_t *left, rectangle_t *right); /*done*/

double rectangle_size(rectangle_t *rect); /*done*/



typedef struct {
    double ysize, xsize;
    size_t rect_alive;
    size_t rect_max;    /*keeps max number of rects therefore can be used by calloc*/
    rectangle_t **rects;
    struct timespec ts_init;
    struct timespec ts_curr;
} plane_t;


void frame_render(plane_t plane); /*done*/

void frame_simulate(plane_t *plane); /*ticks chosen*/

/* these functions are not responsible for rectangle movement simulation,
 * they only change rectangle state as it is dictated by behaviour vars*/
void action_nostim  (plane_t *plane, size_t index); /*partially*/
void action_food    (plane_t *plane, size_t index); /*partially*/
void action_big     (plane_t *plane, size_t index); /*partially*/
void action_prey    (plane_t *plane, size_t index); /*partially*/

void rectangle_hybernate    (plane_t *plane, size_t index); 
void rectangle_move_random  (plane_t *plane, size_t index); 
void rectangle_move_lrandom (plane_t *plane, size_t index); 
void rectangle_move_avoid   (plane_t *plane, size_t index); 
void rectangle_move_seek    (plane_t *plane, size_t index); 

/*this function is used to manage energy spendings for movement*/
void rectangle_accelerate(rectangle_t *rect, double x, double y);

/*returns enum value of action which will be delegated to rectangle*/
enum actions rectangle_action_get(plane_t *plane, size_t index);

/*performs one of N possible actions determined by rectangle_action_get*/
void rectangle_act(plane_t *plane, size_t index);

/* changes rectangle coordinates and resolves collisions for one tick 
 * breaks if index, plane or rectangle is invalid*/
void rectangle_simulate(plane_t *plane, size_t index);

plane_t *plane_create(double xsize, double ysize); /*done*/

/* if rects is NULL, plane will be initialized with random rects */
void plane_init(plane_t *plane, rectangle_t **rects, size_t rects_size); /*done*/

/* need func which conditions plane reset*/
void plane_destroy(plane_t *plane); /*done*/

void plane_remove_rectangle(plane_t *plane, size_t index);

/*returns first collision rectangle index, sets flag true if collision happened*/
size_t plane_check_collisions(plane_t *plane, size_t index, int *flag_collided); 

/* should be called on collision, manages rectagles fight only */
void rectangle_collision_fight(plane_t *plane, size_t left, size_t right); /*done*/

/* completely resolves two rectangles collision recursively 
 * only spends collision tunneled movement
 * it may affect another rectangles */
void rectangle_collision_resolve(plane_t *plane, size_t left); /*done*/

/* checks for and resolves borders collision in this iteration
 * returns true if resolution did happen, false if nothing was changed */
int rectangle_borders_resolve(plane_t *plane, size_t index); /*TODO move code from rectangle_simulate */
