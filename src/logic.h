#ifndef SQUARES_LOGIC_H
#define SQUARES_LOGIC_H

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
//TODO include logger and implement logging interface

/* CONFIG */
/* rectangle and collision */
#define SIZE_DIFF_TRESHOLD          1.2f        /* rect must be ... times bigger in order to eat another rect*/
#define WIDTH_INIT_MIN              2           /* rectangle size at start will be between MIN and MAX*/
#define WIDTH_INIT_MAX              6
#define HEIGHT_INIT_MIN             2           
#define HEIGHT_INIT_MAX             6          
#define SPEED_INIT_MAX              4           /* at game start rectangle speed is not higher than this*/
#define SPEED_ABS_MAX               15          /* highest possible speed */
#define SPEED_ABS_MIN               0           
#define ACCEL_ABS_MAX               8           /* highest possible acceleration */
#define ACCEL_ABS_MIN               0           
#define DETECT_DIST_MAX             50          /* rectangles cant detect objects further than this */
#define DETECT_DIST_MIN             2           /* rectangles will always detect objects at this distance */
#define RECT_DELAY_MAX              20          /* all rectangles internal timer will wait no more than this*/
#define RECT_DELAY_MIN              0.75        /* minimum time until timer triggers */
#define RECTANGLE_INIT_MAX_RETRIES  10          /* stop retrying to spawn rectangle after x attempts*/

/* time */
#define TICK_NSEC                   33333333    /* 5000000LL => 500ticks/sec, number of ns between ticks*/
#define NSEC_IN_SEC                 1000000000LL 
#define BOUNCE_SPEED_FINE           0.4         /* spd = spd - spd*fine*/

/* derivative defines */
#define COLLISION_DELTA             (double)((double)SPEED_ABS_MAX * TICK_NSEC / NSEC_IN_SEC)
#define GLOBAL_TIMER_STEP           TICK_NSEC   /* TODO */

/* INITS */
#define RANDOM_NO_STIM  (unsigned int)(rand() % SE_NUMBER)
#define RANDOM_FOOD     (unsigned int)(rand() % FE_NUMBER)
#define RANDOM_BIG      (unsigned int)(rand() % BE_NUMBER)
#define RANDOM_PREY     (unsigned int)(rand() % PE_NUMBER)
/* DEFAULT TRAITS
 * stored as DOUBLES
 * time in seconds,
 * distance in units,
 * speed in units per second
 * etc...*/

/* IMPORTANT NOTICE
 * speed is given in distance units per second */

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)
#define RAND(min, max) (min + rand() % (max-min+1))
#define DRAND(min, max) (((double)rand() * ( max - min )) / (double)RAND_MAX + min)

#define true  1
#define false 0
#define DELTA 0.000005

/* another rect is same size, bigger or smaller than rect */
enum size_diff_e {sd_same, sd_prey, sd_hunter};

/* these enums determine what rectangle do when
action is chosen| do one of these actions instead*/
enum no_stim    { so_idle, so_random, so_lrandom, SE_NUMBER };
enum food       { fo_idle, fo_random, fo_lrandom, fo_avoid, fo_seek, FE_NUMBER };
enum big        { bo_idle, bo_random, bo_lrandom, bo_avoid, bo_seek, BE_NUMBER };
enum prey       { po_idle, po_random, po_lrandom, po_avoid, po_seek, PE_NUMBER };

enum actions    { a_no_stim, a_food, a_big, a_prey, A_NUMBER }; 

/* traits index kept in enum are used to access items in traits array */
enum traits     { 
    ti_mspeed=0,        ti_accel,           ti_nostim_secs,
    ti_mrandom_delay,   ti_avoid_dist,      ti_avoid_speed,
    ti_pursue_dist,     ti_pursue_speed,    ti_food_dist,
    ti_food_speed, 
TIE_NUMBER };

/* trait names corresponding to traits enum item names for UI*/


/* this type represents selected action's enum number */
/*
typedef struct {
    enum no_stim  nostim_o;
    enum food     food_o;
    enum big      big_o;
    enum prey     prey_o;
} actions_opt_t;
*/

int mutate(int eo_target, int e_number, float rate); 

typedef enum {
    SQ_RED,
    SQ_GREEN,
    SQ_NBLUE
} SQ_COLORS;

typedef struct rectanlge {
    char name[16];                 //FIXME use char array instead
    SQ_COLORS color;
    int frags;
    
    double y, x;
    double height, width;
    double yspeed, xspeed;      /* real rectangle speed at any moment*/
    double angle;               /* direction in which rectangle tries to move */
    double energy;
    double energy_stored;

    double secs_idle;           /* number of seconds w/o stimuli */
    double secs_timer;          /* used by any of action functs*/
    int move_time;              /* ??? */

    unsigned int actions[A_NUMBER];
    enum actions prev_action;
    double traits[TIE_NUMBER];  /* array of traits, max index=TIE_NUMBER*/
    int lock;                   /* pointer to rectangle which rect is locked to*/
} rectangle_t;

struct global_time {
    struct timespec tick_start;
    struct timespec start_time;
    struct timespec lost_time;
};

/* action index size tables */
extern const unsigned int action_enums_size[];

/* action user string tables */
extern const char *action_slot_names[];
/*TODO append action_ to these names*/
extern const char **funames         []; /* contains lower 4 arrays */
extern const char *funames_no_stim  [];
extern const char *funames_food     [];
extern const char *funames_big      [];
extern const char *funames_prey     [];

/* trait permitted values tables */
extern const double trait_min_vals [];
extern const double trait_max_vals [];

/* trait user string tables */
extern const char *trait_names      [];




extern struct global_time GLOBAL_TIME;

/* checks if timer elapsed
 * if elapsed sets timer to current time and returnt true
 * if not returns false*/
//TODO move all duplicate code
void timer_waitreset(struct timespec *start, long long nsecs);
void timer_reset(struct timespec *start);
void timer_wait(struct timespec *start, long long nsecs);
int  timer_check(struct timespec *start, long long nsecs);

void time_begin();
void time_start_logic();
void time_next();
void time_next_nowait();

rectangle_t *rectangle_create(); /*done*/

/* sets all traits to defaults*/
void rectangle_default_traits(rectangle_t *rect);

rectangle_t *rectangle_copy(rectangle_t *rect); /*done*/

void rectangle_destroy(rectangle_t *rect); /*done*/

int rectangle_set_action(rectangle_t *rect, int index, double val); /*done*/

int rectangle_set_trait(rectangle_t *rect, int index, double val);  /*done*/

int rectangle_check_collision(rectangle_t *left, rectangle_t *right); /*done*/

/* modifies rectangle so that it is now dest_size times bigger than original */
void rectangle_resize_x(rectangle_t *rect, double ratio); /*done*/
void rectangle_resize_y(rectangle_t *rect, double ratio); /*done*/


/* return adress of winning rect,
 * if none returns NULL */
rectangle_t *rectangle_compare(rectangle_t *left, rectangle_t *right); /*done*/

size_t rectangle_represent(rectangle_t *rect, char **buff); /*done*/
size_t rectangle_represent_fields(rectangle_t *rect, char **buff);  /*done*/
size_t rectangle_represent_actions(rectangle_t *rect, char **buff); /*done*/
size_t rectangle_represent_traits(rectangle_t *rect, char **buff);  /*done*/

size_t enumerate_actions(char **buf);   /*done*/
size_t enumerate_action_values(char **buf, int action_index);   /*done*/
size_t enumerate_traits(char **buf);    /*done*/
double rectangle_size(rectangle_t *rect); /*done*/

typedef struct {
    double ysize, xsize;
    int rect_alive;
    int rect_max;    /*keeps max number of rects therefore can be used by calloc*/
    rectangle_t **rects;
    struct timespec ts_init;
    struct timespec ts_curr;
} plane_t;
/*
extern void (** const action_functs      [])(plane_t *, int);
extern void (* const action_nostim_functs[])(plane_t *, int);
extern void (* const action_food_functs  [])(plane_t *, int);
extern void (* const action_big_functs   [])(plane_t *, int);
extern void (* const action_prey_functs  [])(plane_t *, int);
*/

void frame_render(plane_t plane); /*done*/

void frame_simulate(plane_t *plane); /*ticks chosen*/

/* these functions are not responsible for rectangle movement simulation,
 * they only change rectangle state as it is dictated by behaviour vars*/
void action_nostim  (plane_t *plane, int index); /*partially*/
void action_food    (plane_t *plane, int index); /*partially*/
void action_big     (plane_t *plane, int index); /*partially*/
void action_prey    (plane_t *plane, int index); /*partially*/

void rectangle_hibernate    (plane_t *plane, int index); 
void rectangle_move_random  (plane_t *plane, int index); 
void rectangle_move_lrandom (plane_t *plane, int index); 
void rectangle_move_avoid   (plane_t *plane, int index); 
void rectangle_move_seek    (plane_t *plane, int index); 

/* this function returns acceleration at current tick 
 * it also calculates energy spendings for acceleration
 * WARN function does not perform speed limit checks*/
void rectangle_accelerate(rectangle_t *rect, double speed, double angle, double accel);

/*returns enum value of action which will be delegated to rectangle*/
enum actions rectangle_action_get(plane_t *plane, int index); /*done*/

/*performs one of N possible actions determined by rectangle_action_get*/
void rectangle_act(plane_t *plane, int index); /*done*/

/* changes rectangle coordinates and resolves collisions for one tick 
 * breaks if index, plane or rectangle is invalid*/
void rectangle_simulate(plane_t *plane, int index); /*done*/

/* returns distance between two rectangles' *centers* */
double rectangle_distance(rectangle_t *left, rectangle_t *right); /*TODO better estimate*/

plane_t *plane_create(double xsize, double ysize); /*done*/

/* if rects is NULL, plane will be initialized with random rects */
void plane_init(plane_t *plane, rectangle_t **rects, int rects_size); /*done*/
void plane_populate_randomly(plane_t *plane, int rects_number); /*done*/
void plane_populate_recombinate(plane_t *plane, rectangle_t **rects, int rects_number);
void plane_populate_alive(plane_t *plane, rectangle_t **rects, int rects_number);

/* need func which conditions plane reset*/
void plane_destroy(plane_t *plane); /*done*/

void plane_remove_rectangle(plane_t *plane, int index); /*done*/

/* returns first collision rectangle index, sets flag true if collision happened*/
int plane_check_collisions(plane_t *plane, int index); /*FIXME*/

/* returns index of first rectanle which matches threshold */
int plane_get_proximate_rectangle(plane_t *plane, int index, double mindist, enum size_diff_e type); /*done*/

/* should be called on collision, manages rectagles fight only */
void rectangle_collision_fight(plane_t *plane, int left, int right); /*done*/

/* completely resolves two rectangles collision recursively 
 * only spends collision tunneled movement
 * it may affect another rectangles */
void rectangle_collision_resolve(plane_t *plane, int left); /*done*/

/* checks for and resolves borders collision in this iteration
 * returns true if resolution did happen, false if nothing was changed */
int rectangle_borders_resolve(plane_t *plane, int index); /*TODO move code from rectangle_simulate */

/* safely returns true only if rectangle is alive, else return 0 */
int plane_is_rect_alive(plane_t *plane, int index);

#endif
