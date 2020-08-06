#include "logic.h"
#include <math.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <time.h>

struct global_time GLOBAL_TIME;

/* VERY IMPORTANT
 * jump and string tables below must ALWAYS be up to date with corresponding enums in header file
 * failing to keep them up to date will cause wrong function names at user output and 
 * will cause wrong functions to be called for rectangle actions */

const unsigned int action_enums_size[] = {
    SE_NUMBER, FE_NUMBER, BE_NUMBER, PE_NUMBER
};
const char* action_slot_names[] = { 
    "no stim", "food", "run", "hunt" 
}; 
const char *funames_no_stim  [] = { "idle", "move random", "long random move" };
const char *funames_food     [] = { "idle", "move random", "long random move", "avoid", "seek" };
const char *funames_big      [] = { "idle", "move random", "long random move", "avoid", "seek" };
const char *funames_prey     [] = { "idle", "move random", "long random move", "avoid", "seek" };
const char **funames         [] = { funames_no_stim, funames_food, funames_big, funames_prey };

const double trait_min_vals [] = { 
    /*0 because speed cant be < 0*/
    SPEED_ABS_MIN,      ACCEL_ABS_MIN,      RECT_DELAY_MIN,
    RECT_DELAY_MIN,     DETECT_DIST_MIN,    SPEED_ABS_MIN,
    DETECT_DIST_MIN,    SPEED_ABS_MIN,      DETECT_DIST_MIN,
    SPEED_ABS_MIN 
};

const double trait_max_vals [] = {
    SPEED_ABS_MAX,      ACCEL_ABS_MAX,      RECT_DELAY_MAX,
    RECT_DELAY_MAX,     DETECT_DIST_MAX,    SPEED_ABS_MAX,
    DETECT_DIST_MAX,    SPEED_ABS_MAX,      DETECT_DIST_MAX,
    SPEED_ABS_MAX 
};

const double trait_min_init_vals[] = {
    1,  0.5,    1,
    1,  3,      3,
    5,  3.25,   3,
    2 
};

const double trait_max_init_vals[] = {
    5,  3,      3,
    2,  8,      6,
    9,  6.25,   7,
    6 
};

const char *trait_names[] = {
    "movement speed", "acceleration", "seconds to idle",
    "random move delay", "avoid distance", "avoid speed",
    "pursue distance", "pursue speed", "food detection distance",
    "food pursue speed",
"traits number" };




/* jump tables for each action, 
 * this one is for rectangle->action[a_no_stim][key], key>0, key<SE_NUMBER*/
static void (* const action_nostim_functs[])(plane_t *, int) = {
    rectangle_hibernate,
    rectangle_move_random,
    rectangle_move_lrandom
};

/* duplicate code because during later development I plan to have
 * independent functions for each actions 
 * I implemented jump logic now, so I can separate them later */

/* notice the difference between [] on this function and *const*const on 3 below
 * if you want to change one of them remove *const and add [] 
 * (define as array and initialize, not as a const pointer)*/
static void (* const action_default_functs  [])(plane_t *, int) = {  
    rectangle_hibernate,
    rectangle_move_random,
    rectangle_move_lrandom,
    rectangle_move_avoid,
    rectangle_move_seek
};

static void (*const *const action_food_functs)(plane_t *, int) = action_default_functs;
static void (*const *const action_big_functs )(plane_t *, int) = action_default_functs;
static void (*const *const action_prey_functs)(plane_t *, int) = action_default_functs;


/* array of constant local jump tables to 
 * this one is for rectangle->action[nact][key]: nact >0, nact <A_NUMBER; key>0, key<action_enums_size[nact]*/
static void (*const *const action_functs[])(plane_t *, int) = {
    action_nostim_functs,
    action_food_functs,
    action_big_functs,
    action_prey_functs
};

void rectangle_random_name(rectangle_t *rect) {
    int bufsize = sizeof(rect->name);
    char is_consonant[bufsize];
    int i;
    char consonants[] = "rtpsdfghklcvbnm";
    char sonants[] = "eyuioa";

    int consize = sizeof(consonants) - 1;
    int sonsize = sizeof(sonants) - 1;

    int min_len = 3;
    int max_seq_consonants = 2;
    int max_seq_sonants = 2;
    int percent_sonant = 35;
    int term_percent = 35;  /*at every char*/

    for (i = 0; i < bufsize - 1; i++) {
        if (i > min_len  && (rand() % 100 < term_percent) ) {
            rect->name[i] = '\0';
            break;
        }

        if (i >= max_seq_sonants) {
            int j;
            int force_consonant = 1;
            for (j = 1; j <= max_seq_sonants; j++) {
                if (is_consonant[i-j]) {
                    force_consonant = 0;
                    break;
                }
            }

            if (force_consonant) {
                is_consonant[i] = 1;
                rect->name[i] = consonants[rand() % consize];
                continue;
            }
        }

        if (i >= max_seq_consonants) {
            int j;
            int force_sonant = 1;
            for (j = 1; j <= max_seq_consonants; j++) {
                if (!is_consonant[i-j]) {
                    force_sonant = 0;
                    break;
                }
            }

            if (force_sonant) {
                is_consonant[i] = 0;
                rect->name[i] = sonants[rand() % sonsize];
                continue;
            }
        }

        if (rand() % 100 < percent_sonant) {
            is_consonant[i] = 0;
            rect->name[i] = sonants[rand() % sonsize];
        } else {
            is_consonant[i] = 1;
            rect->name[i] = consonants[rand() % consize];
        }
    }

    rect->name[0] = (char)toupper(rect->name[0]);
    rect->name[bufsize - 1] = '\0';
}

void rectangle_default_traits(rectangle_t *rect){
    int i;
    for (i = 0; i < TIE_NUMBER; i++) {
        double min, max;
        min = trait_min_init_vals[i];
        max = trait_max_init_vals[i];
        rect->traits[i] = DRAND(min, max);
    }
}

rectangle_t *rectangle_create() {
    rectangle_t *rect = (rectangle_t*)calloc(1, sizeof(rectangle_t));

    rect->actions[a_no_stim ] = RANDOM_NO_STIM; 
    rect->actions[a_food    ] = RANDOM_FOOD; 
    rect->actions[a_big     ] = RANDOM_BIG; 
    rect->actions[a_prey    ] = RANDOM_PREY; 
    
    rect->energy = 100;
    rectangle_default_traits(rect);
    rectangle_random_name(rect);
    return rect;
}

void rectangle_destroy(rectangle_t *rect) {
    free(rect);
}

//TODO they should do some checks and updates of rectangle state
void rectangle_resize_x(rectangle_t *rect, double ratio) { 
    //if ( !ratio ) return;
    rect->width *= ratio;
}

void rectangle_resize_y(rectangle_t *rect, double ratio) { 
    //if ( !ratio ) return;
    rect->height *= ratio;
}

rectangle_t *rectangle_copy(rectangle_t *rect) {
    rectangle_t *copy = rectangle_create();
    memcpy(copy, rect, sizeof(rectangle_t));
    return copy;
}

/* val is double only to keep signature of all setters same */
int rectangle_set_action(rectangle_t *rect, int index, double val) {
    unsigned int a_index = (unsigned int)val;
    if (!rect) return 0;
    if (index >= A_NUMBER || index < 0)                  return 0;
    if ( a_index >= action_enums_size[index]) return 0;

    rect->actions[index] = a_index;
    return 1;
}
//TODO
int rectangle_set_trait(rectangle_t *rect, int index, double val) {
    if (!rect) return 0;
    if (index >= TIE_NUMBER || index < 0) return 0;
    if (val > trait_max_vals[index] || val < trait_min_vals[index]) return 0;

    rect->traits[index] = val;
    return 1;
}

void rectangle_collision_fight(plane_t *plane, int left, int right) {
    if (left == right) return; 

    rectangle_t *lrect, *rrect;
    lrect = plane->rects[left];
    rrect = plane->rects[right]; 

    rectangle_t *winner = rectangle_compare(lrect, rrect);

    if (!winner) {
        if (!(rand() % 10)) 
            return;
        if (rand() % 2) {
            rectangle_resize_x(lrect, 0.9f);
            rectangle_resize_y(rrect, 0.9f);
        } else {
            rectangle_resize_y(lrect, 0.9f);
            rectangle_resize_x(rrect, 0.9f);
        }
    }
    
    //TODO
    /* some genetic logic may be defined here
     * for now just kill smaller one */
    int looser = (winner == lrect) ? right : left;

    plane_remove_rectangle(plane, looser);

    return;
}

double rectangle_speed_get(rectangle_t *rect) {
    return sqrt(rect->xspeed * rect->xspeed + rect->yspeed * rect->yspeed);
}

double rectangle_get_angle(rectangle_t *rect, rectangle_t *target) {
	double x1, x2, y1, y2;
    x1 = rect->x;
    y1 = rect->y;

    x2 = target->x;
    y2 = target->x;
    
    return atan2(y2 - y1, x2 - x1) - 3.1415/2;
}

double dot_distance(double x1, double y1, double x2, double y2) {
    double dx, dy;
    double sx, sy;
    dx = x2 - x1;
    dy = y2 - y1;
    sx = dx * dx;
    sy = dy * dy;
    return sqrt(sx + sy);
}

/*speed units/sec, angle radians (0 - down, Pi/2 - left, Pi - up, ...), accel units/sec^2*/
void rectangle_accelerate(rectangle_t *rect, double speed, double angle, double accel) {
    angle = fmod(angle, 2*M_PI);
    if (angle < 0) angle = M_PI*2 + angle;
    int dl  = angle >= 0        && angle <= M_PI/2,
        ul  = angle > M_PI/2    && angle <= M_PI,
        ur  = angle > M_PI      && angle <= M_PI*1.5,
        dr  = angle > M_PI*1.5  && angle <= M_PI*2;
    /* calculate destination speed vector coordinates */
    double x_dst, y_dst;
    if (dl) {
        x_dst = -speed * sin(angle);
        y_dst =  speed * cos(angle);
    } else
    if (ul) {
        angle -= M_PI/2;
        x_dst = -speed * cos(angle);
        y_dst = -speed * sin(angle);
    } else
    if (ur) {
        angle -= M_PI;
        x_dst =  speed * sin(angle);
        y_dst = -speed * cos(angle);
    } else
    if (dr) {
        angle -= M_PI*1.5;
        x_dst = speed * sin(angle);
        y_dst = speed * cos(angle);
    } else exit(1); //TODO ASSERT

    double d_sec;
    d_sec = (double)TICK_NSEC / NSEC_IN_SEC;

    double dx, dy;
    double accel_left;
    const double 
        x_diff = x_dst - rect->xspeed, 
        y_diff = y_dst - rect->yspeed;
    const double
        x_diff_abs = fabs(x_diff),
        y_diff_abs = fabs(y_diff);
    accel_left = accel * d_sec;

    int is_x_complete, is_y_complete;
    is_x_complete = is_y_complete = false;
    
    if (x_diff_abs <= accel_left) {
        accel_left -= x_diff_abs;
        dx = x_diff;
        is_x_complete = true;
    }

    if (y_diff_abs <= accel_left) {
        accel_left -= y_diff_abs;
        dy = y_diff;
        is_y_complete = true;
    }

    if (!is_x_complete && !is_y_complete) {
        dx = copysign(accel_left/2, x_diff);
        dy = copysign(accel_left/2, y_diff);
        is_x_complete = is_y_complete = true;
    }

    if (!is_x_complete) dx = copysign(accel_left, x_diff);
    if (!is_y_complete) dy = copysign(accel_left, y_diff);

    rect->xspeed += dx;
    rect->yspeed += dy;

    /* TODO calculate energy losses based on dx, dy */
}

void rectangle_TESTMOVE     (plane_t *plane, int index) { 
    //FIXME redirecting to move_random
    rectangle_move_random(plane, index);
    return;
}


/* TODO rectangle_drift action: dont change speed at all
 * TODO traits must be kept up to date by another function
 * it must be called on initialize or traits change 
 * like: if (speed > SPEED_ABS_MAX) speed = SPEED_ABS_MAX; 
 * */
void rectangle_hibernate    (plane_t *plane, int index) { 
    double angle, accel;
    rectangle_t *rect = plane->rects[index];
    accel = rect->traits[ti_accel];
    angle = rect->angle;

    rectangle_accelerate(rect, angle, SPEED_ABS_MIN, accel);
}

void rectangle_move_random  (plane_t *plane, int index) { 
    rectangle_t *rect = plane->rects[index];
    double angle = rect->angle;
    if (rect->secs_timer >= rect->traits[ti_mrandom_delay]) {
        angle = ((double)rand() / RAND_MAX) * 2 * M_PI;
        rect->secs_timer = 0;
        rect->angle = angle;
    }

    double speed = rect->traits[ti_mspeed];
    rectangle_accelerate(rect, speed, angle, rect->traits[ti_accel]);
}

void rectangle_move_lrandom (plane_t *plane, int index) { rectangle_TESTMOVE(plane, index); }

void rectangle_move_avoid   (plane_t *plane, int index) { 
    double angle, speed, accel;
    rectangle_t *rect, *target;
    rect = plane->rects[index];
    target = plane->rects[rect->lock];
    if (!target) return;

    /* opposite angle */
    angle = rectangle_get_angle(rect, target) + M_PI;
    accel = rect->traits[ti_accel];
    speed = rect->traits[ti_avoid_speed];

    rectangle_accelerate(rect, speed, angle, accel);
}

void rectangle_move_seek    (plane_t *plane, int index) {
    double angle, speed, accel;
    rectangle_t *rect, *target;
    rect = plane->rects[index];
    target = plane->rects[rect->lock];
    if (!target) return;

    angle = rectangle_get_angle(rect, target);
    accel = rect->traits[ti_accel];
    speed = rect->traits[ti_pursue_speed];

    rectangle_accelerate(rect, speed, angle, accel);
}

/* FIXME deprecated */
void timer_wait(struct timespec *start, long long nsecs) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long long nsec_elapsed = (ts.tv_sec - start->tv_sec) * NSEC_IN_SEC
                                    + ts.tv_nsec - start->tv_nsec;

    if (nsec_elapsed < nsecs) {
        //clock_gettime(CLOCK_REALTIME, &start);
        long long wait_nsec = nsecs - nsec_elapsed;
        ts.tv_sec  = 0;
        while (wait_nsec >= NSEC_IN_SEC) {
            ts.tv_sec++;
            wait_nsec -= NSEC_IN_SEC;
        }
        ts.tv_nsec = wait_nsec;
        nanosleep(&ts, NULL);
    }
}

void timer_reset(struct timespec *start) {
    clock_gettime(CLOCK_REALTIME, start);
}

void timer_waitreset(struct timespec *start, long long nsecs) {
    timer_wait(start, nsecs);
    timer_reset(start);
}

int timer_check(struct timespec *start, long long nsecs) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long long nsec_elapsed = (ts.tv_sec - start->tv_sec) * NSEC_IN_SEC
                                    + ts.tv_nsec - start->tv_nsec;


    return (nsec_elapsed < nsecs);
}

void time_begin() {
    timer_reset(&GLOBAL_TIME.start_time);
    GLOBAL_TIME.lost_time.tv_sec  = 0;
    GLOBAL_TIME.lost_time.tv_nsec = 0;
}

void time_start_logic() {
    timer_reset(&GLOBAL_TIME.tick_start);
}

void time_next() {
    struct timespec ts;
    long long nsecs;

    clock_gettime(CLOCK_REALTIME, &ts);

    /* elapsed time */
    nsecs = 0;
    ts.tv_sec -= GLOBAL_TIME.tick_start.tv_sec;
    nsecs += ts.tv_sec * NSEC_IN_SEC;
    nsecs += ts.tv_nsec - GLOBAL_TIME.tick_start.tv_nsec;

    /* tick took longer than TICK_NSEC */
    if (nsecs >= TICK_NSEC) {
        GLOBAL_TIME.lost_time.tv_sec += nsecs / NSEC_IN_SEC;
        GLOBAL_TIME.lost_time.tv_nsec += nsecs % NSEC_IN_SEC;
        if (GLOBAL_TIME.lost_time.tv_nsec >= NSEC_IN_SEC) {
            GLOBAL_TIME.lost_time.tv_sec++;
            GLOBAL_TIME.lost_time.tv_nsec -= NSEC_IN_SEC;
        }
        return;
    }

    nsecs = TICK_NSEC - nsecs;

    ts.tv_sec  = nsecs / NSEC_IN_SEC;
    ts.tv_nsec = nsecs % NSEC_IN_SEC;

    nanosleep(&ts, NULL);
}

void frame_simulate(plane_t *plane) {
    int max = plane->rect_max;
    int i;
    for (i = 0; i < max; i++) {
        if (!plane->rects[i]) continue;
        rectangle_act(plane, i);
    }
}

enum actions rectangle_action_get(plane_t *plane, int index) {
    rectangle_t *rect = plane->rects[index];
    int target;
    target = plane_get_proximate_rectangle(plane, index, rect->traits[ti_avoid_dist], sd_hunter);
    if (target != -1) {
        rect->lock = target;
        return a_big;
    }

    target = plane_get_proximate_rectangle(plane, index, rect->traits[ti_pursue_dist], sd_prey);
    if (target != -1) {
        rect->lock = target;
        return a_prey;
    }
    //TODO food
    
    return a_no_stim;
}

void rectangle_act(plane_t *plane, int index) {
    rectangle_t *rect = plane->rects[index];
    enum actions action = rectangle_action_get(plane, index);

    /* update rectangle timers */
    double dtime = (double)TICK_NSEC / (double)NSEC_IN_SEC;
    rect->secs_timer += dtime;

    /* switch to a_no_stim if rect did not get any other action during ti_nostim_secs */
    if (action == a_no_stim){
        rect->secs_idle += dtime;
        double delay = rect->traits[ti_mrandom_delay];
        if (rect->secs_idle >= delay) {
            rect->secs_idle = 0;
            action = a_no_stim;
        } else {
            action = rect->prev_action;
        }
    }

    //* rectangle->action[nact][key]: nact >0, nact <A_NUMBER; key>0, key<action_enums_size[nact]*/
    if (action < 0 || action >= A_NUMBER) {
        /* invalid action index*/
        fprintf(stderr, "at logic.c - rectangle_act - invalid action variable\n");
        exit(1);
    }

    unsigned int fkey = rect->actions[action];
    if (fkey >= action_enums_size[action]) {
        /* invalid action slot index for action variable */
        fprintf(stderr, "at logic.c - rectangle_act - invalid fkey\n");
        exit(1);
    }

    action_functs[action][fkey](plane, index);

    rectangle_simulate(plane, index);
    if (plane->rects[index]) rect->prev_action = action;
}

int rectangle_borders_resolve(plane_t *plane, int index) {
    //FIXME MOVE CODE
    rectangle_t *r = plane->rects[index];
    /* up */
    if (r->y < 0 ) {
        double overflow = r->y - 0; 
        r->y -= 2*overflow;
        r->yspeed = -r->yspeed;
        return 1;
    } 
    /* bot */
    else if (r->y + r->height > plane->ysize) {
        double overflow = r->y + r->height - plane->ysize;
        r->y -= 2*overflow;
        r->yspeed = -r->yspeed;
        return 1;
    }
    /* left */
    if (r->x < 0) {
        double overflow = r->x - 0;
        r->x -= 2*overflow;
        r->xspeed = -r->xspeed;
        return 1;
    }
    /* right */
    else if (r->x + r->width > plane->xsize) { 
        double overflow = r->x + r->width - plane->xsize;
        r->x -= 2*overflow;
        r->xspeed = -r->xspeed;
        return 1;
    }
    return 0;
}

static inline double col_il(rectangle_t *r, rectangle_t *t) {
    double xovershoot = r->x - t->y - t->width; //<0 checked
    r->xspeed = -r->xspeed;
    t->xspeed = -t->xspeed;

    return xovershoot;
}

static inline double col_ir(rectangle_t *r, rectangle_t *t) {
    double xovershoot = r->x + r->width - t->width; //>0 checked
    r->xspeed = -r->xspeed;
    t->xspeed = -t->xspeed;

    return xovershoot;
}

static inline double col_iu(rectangle_t *r, rectangle_t *t) {
    double yovershoot = r->y - (t->y + t->height); //<0 checked
    r->yspeed = -r->yspeed;
    t->yspeed = -t->yspeed;
    
    return yovershoot;
}

static inline double col_id(rectangle_t *r, rectangle_t *t) {
    double yovershoot = r->y + r->height - t->y; //>0 checked
    r->yspeed = -r->yspeed;
    t->yspeed = -t->yspeed;

    return yovershoot;
}

static inline void col_apply(rectangle_t *r, double *xovershoot, double *yovershoot) {
    if (fabs(*xovershoot) > COLLISION_DELTA) {
        *xovershoot = 0;
    }
    if (fabs(*yovershoot) > COLLISION_DELTA) {
        *yovershoot = 0;
    } 

    r->x -= 2* *xovershoot;
    r->y -= 2* *yovershoot;
}

/* lol clear bloat */
/*if rectangle gets teleported out of plane, program crashes*/
void rectangle_collision_resolve(plane_t *plane, int index) {
    if (!plane->rects[index]) return; /* rectangle was killed, exit recursion */
    int col_index = plane_check_collisions(plane, index);
    /* if no rect collision check borders */
    if (col_index == -1) {
        /* if state changed go deeper */
        if(rectangle_borders_resolve(plane, index))
            rectangle_collision_resolve(plane, index);
        return; 
        /* no rect collision and no border collision exit recursion 
         * or terminate exiting recursion */
    }

    /* rect collision happened, resolve by tunneling distance then go deeper */
    /* go deeper */
    rectangle_t *r = plane->rects[index];
    rectangle_t *t = plane->rects[col_index];
    /* i[udlrvh] flags indicate if intersection from [udlrvh] side is possible
     * udlr - up, down, left, right; strictly
     * vh   - vertically, horizontally */
    int iu, id, il, ir, iv, ih;
    /* WARN these variables duplicate in rectangle_check_collision 
     * TODO rectangle_check_collision should return enum of collision direction
     * to remove duplicate code. This function should use provided enums 
     * instead of making checks itself
     *
     * TODO better speed change than just inversion
     * TODO subtract COLLISION_DELTA from greater overshoot coordinate respecting sign */
    iu = r->y >= t->y && r->y <= t->y + t->height;
    id = r->y + r->height >= t->y && r->y + r->height <= t->y + t->height;
    il = r->x >= t->x && r->x <= t->x + t->width;
    ir = r->x + r->width >= t->x && r->x + r->width <= t->x + t->width;

    iv = (id) || (r->y <= t->y && r->y + r->height >= t->y + t->height) || (iu);
    ih = (il) || (r->x <= t->x && r->x + r->width >= t->x + t->width)   || (ir);


    double xovershoot = 0, 
           yovershoot = 0;

    /* UP */
    if (iu && ih) {
        /* X overshoot */
        if (il) { 
            xovershoot = col_il(r, t);
        } else if (ir) {
            xovershoot = col_ir(r, t);
        }
        /* Y overshoot */
        yovershoot = col_iu(r, t);
        col_apply(r, &xovershoot, &yovershoot);
        rectangle_collision_fight(plane, index, col_index);
    } 
    /* DOWN */
    else if (id && ih) {
        /* X overshoot*/
        if (il) { 
            xovershoot = col_il(r, t);
        } else if (ir) {
            xovershoot = col_ir(r, t);
        }
        /* Y overshoot */
        yovershoot = col_id(r, t);
        col_apply(r, &xovershoot, &yovershoot);
        rectangle_collision_fight(plane, index, col_index);
    }     
    /* LEFT */
    else if (il && iv) {
        /* Y overshoot */
        if (iu) {
            yovershoot = col_iu(r, t);
        } else if (id) {
            yovershoot = col_id(r, t);
        }
        /* X overshoot*/
        xovershoot = col_il(r, t);
        col_apply(r, &xovershoot, &yovershoot);
        rectangle_collision_fight(plane, index, col_index);
    }
    /* RIGHT */
    else if (ir && iv) {
        if (iu) {
            yovershoot = col_iu(r, t);
        } else if (id) {
            yovershoot = col_id(r, t);
        }
        /* X overshoot*/
        xovershoot = col_ir(r, t);
        col_apply(r, &xovershoot, &yovershoot);
        rectangle_collision_fight(plane, index, col_index);
    }

    return;
}

void rectangle_simulate(plane_t *plane, int index) {
    rectangle_t *rect = plane->rects[index];
    double nx, ny;
    nx = rect->x + (rect->xspeed) * (double)((long double)TICK_NSEC/(long double)NSEC_IN_SEC);
    ny = rect->y + (rect->yspeed) * (double)((long double)TICK_NSEC/(long double)NSEC_IN_SEC);
    rect->x = nx;
    rect->y = ny;

    //FIXME this function does not yet respect rectangle actions
    
    /* continue checks untill resolved, 
     * if no changes were made during iteration, unset unresolved flag */
    //FIXME move code to borders_resolve
    rectangle_collision_resolve(plane, index);
}

rectangle_t *rectangle_compare(rectangle_t *left, rectangle_t *right) { 
    double  lsize, rsize;
    lsize = rectangle_size(left),
    rsize = rectangle_size(right);
    double delta = MAX(lsize, rsize) / MIN(rsize, lsize); 
    if ( delta < SIZE_DIFF_TRESHOLD ) return NULL;
    return MAX(rsize, lsize) == rsize ? right : left;
}

//TODO clean
/* writes rectangle text representation to buffer 
 * returns buffer size or 0 on fail
 * size does NOT include \0 */

size_t rectangle_represent_fields(rectangle_t *rect, char **buff) {
    size_t size;
    if (!rect) return 0;

    FILE *mstream = open_memstream(buff, &size);
    if (!mstream) return 0;
    
    fprintf(mstream, 
        "    NAME [%s]\nxspd %f, yspd %f, angle %frad\ny %f, x %f, height %f, width %f\nenrg %f",
        rect->name, 
        rect->xspeed, rect->yspeed, rect->angle, 
        rect->y, rect->x, rect->height, rect->width,
        rect->energy
    );

    fclose(mstream);
    return size;
}

size_t rectangle_represent_actions(rectangle_t *rect, char **buff) {
    size_t size;
    if (!rect) return 0;

    FILE *mstream = open_memstream(buff, &size);
    if (!mstream) return 0;

    fprintf(mstream, "    ACTIONS\n");
    size_t i;
    /* print each action slot and its function */
    for (i = 0; i < A_NUMBER; i++) {
        fprintf(mstream, "%zu %-19s %s", 
            i, action_slot_names[i], funames[i][rect->actions[i]]);
        if (i + 1 != A_NUMBER) fprintf(mstream, "\n");
    }

    fclose(mstream);
    return size;
}

size_t rectangle_represent_traits(rectangle_t *rect, char **buff) {
    size_t size;
    if (!rect) return 0;

    FILE *mstream = open_memstream(buff, &size);
    if (!mstream) return 0;

    fprintf(mstream, "    TRAITS\n");
    size_t i;
    for (i = 0; i < TIE_NUMBER; i++) {
        fprintf(mstream, "%zu %-23s %f", 
            i, trait_names[i], rect->traits[i]);
        if (i + 1 != TIE_NUMBER) fprintf(mstream, "\n");
    }

    fclose(mstream);
    return size;
}
size_t rectangle_represent(rectangle_t *rect, char **buff) {
    size_t size;
    if (!rect) return 0;

    FILE *mstream = open_memstream(buff, &size);
    if (!mstream) return 0;

    char *fields, *actions, *traits;
    rectangle_represent_fields(rect, &fields);
    rectangle_represent_actions(rect, &actions);
    rectangle_represent_traits(rect, &traits);

    fprintf(mstream, "%s\n%s\n%s", 
        fields, actions, traits);

    free(fields);
    free(actions);
    free(traits);

    fclose(mstream);
    return size;
}

size_t enumerate_actions(char **buf) {
    size_t size;
    FILE *mstream;
    mstream = open_memstream(buf, &size);
    if (!mstream) return 0;

    int i;
    for (i = 0; i < A_NUMBER; i++) {
        fprintf(mstream, "%d %s", 
            i, action_slot_names[i]);
        if (i != A_NUMBER - 1) fprintf(mstream, "\n");
    }

    fclose(mstream);
    return size;
}

size_t enumerate_action_values(char **buf, int action_index) {
    if (action_index < 0 || action_index >= A_NUMBER) return 0;

    size_t size;
    FILE *mstream;
    mstream = open_memstream(buf, &size);
    if (!mstream) return 0;

    fprintf(mstream, "Possible values of [%s]\n", action_slot_names[action_index]);

    size_t i;
    size_t action_size = action_enums_size[action_index];
    for (i = 0; i < action_size; i++) {
        fprintf(mstream, "%zu %s", 
            i, funames[action_index][i]);
        if (i != action_size - 1) fprintf(mstream, "\n");
    }

    fclose(mstream);
    return size;
}

size_t enumerate_traits(char **buf) {
    size_t size;
    FILE *mstream;
    mstream = open_memstream(buf, &size);
    if (!mstream) return 0;

    int i;
    for (i = 0; i < TIE_NUMBER; i++) {
        fprintf(mstream, "%d %s", 
            i, trait_names[i]);
        if (i != TIE_NUMBER - 1) fprintf(mstream, "\n");
    }

    fclose(mstream);
    return size;

}


double rectangle_size(rectangle_t *rect) { 
    return (double)(rect->width * rect->height);
}

/* initialize rectangle_t rect fields with random values*/
void rectangle_relocate_randomly(plane_t *plane, rectangle_t *rect) {
    rect->width     = rand() % (WIDTH_INIT_MAX - WIDTH_INIT_MIN + 1) + WIDTH_INIT_MIN;
    rect->height    = rand() % (HEIGHT_INIT_MAX - HEIGHT_INIT_MIN + 1) + HEIGHT_INIT_MIN;
    rect->x         = rand() % (int)(plane->xsize - rect->width);
    rect->y         = rand() % (int)(plane->ysize - rect->height);
    //TODO initialize actions randomly
}

static void plane_empty(plane_t *plane) {
    if (plane->rects) {
        int i;
        for (i = 0; i < plane->rect_max; i++) {
            if (plane->rects[i]) rectangle_destroy(plane->rects[i]);
        }
        free(plane->rects);
    }
    plane->rect_max = 0;
    plane->rect_alive = 0;
    timer_reset(&plane->ts_init);
    timer_reset(&plane->ts_curr);
}

void plane_populate_randomly(plane_t *plane, int rects_number) {
    plane_empty(plane);
    plane->rects = calloc(sizeof(rectangle_t*), (size_t)rects_number);
    plane->rect_max = rects_number;
    plane->rect_alive = rects_number;
    int i;
    for (i = 0; i < rects_number; i++) {
        rectangle_t *rect = plane->rects[i] = rectangle_create();
        rectangle_relocate_randomly(plane, rect);
        int j = 0;
        int col_index;
        while (j++ < RECTANGLE_INIT_MAX_RETRIES) { 
            col_index = plane_check_collisions(plane, i);
            if (!col_index) break;
            rectangle_relocate_randomly(plane, rect);        
        }
        if (j == RECTANGLE_INIT_MAX_RETRIES) {
            plane_remove_rectangle(plane, i);
        }
    }
    /* TODO beter initialization*/
}

void plane_populate_recombinate(plane_t *plane, rectangle_t **rects, int rects_number);

plane_t *plane_create(double xsize, double ysize) {
    plane_t *plane = calloc(sizeof(plane_t), 1);
    plane->xsize = xsize;
    plane->ysize = ysize;
    return plane;
}

void plane_destroy(plane_t *plane) {
    if (!plane) return;
    int i, max;
    max = plane->rect_max;
    rectangle_t **rects = plane->rects;
    if (rects) {
        for(i = 0; i < max; i++) {
            if (!rects[i]) continue;
            rectangle_destroy(rects[i]);
        }
        free(rects);
    }
    free(plane);
}


/* Checks for collisions for rectangle of given index 
 * WARN assumes that index is valid 
 * this function does not check for NULL rectangles at index 
 * return int index of collision rectangle
 * return -1 if not collided
 * FIXME complexity O(N) for each rectangle, O(N**2) per tick
 * TODO keep rectangles in tree data structure or divide planes into sectors */
int plane_check_collisions(plane_t *plane, int index) {
    int i;
    rectangle_t *rect = plane->rects[index];
    for (i = 0; i < plane->rect_max; i++) {
        rectangle_t *curr = plane->rects[i];
        if (!curr) continue;
        if (rectangle_check_collision(curr, rect) && rect != curr) {
            return i;
        }
    }
    return -1;
}

/* WARN function assumes that left and right exist and were initialized correctly*/
int rectangle_check_collision(rectangle_t *l, rectangle_t *r) {
    /*TODO return enum of collision direction instead*/
    int iu, id, il, ir, iv, ih;
    iu = l->y >= r->y && l->y <= r->y + r->height;
    id = l->y + l->height >= r->y && l->y + l->height <= r->y + r->height;
    iv = (id) || (l->y <= r->y && l->y + l->height >= r->y + r->height) || (iu);
    if (!iv) return 0;

    il = l->x >= r->x && l->x <= r->x + r->width;
    ir = l->x + l->width >= r->x && l->x + l->width <= r->x + r->width;
    ih = (il) || (l->x <= r->x && l->x + l->width >= r->x + r->width)   || (ir);
    if (!ih) return 0;

    return 1;
}


/* properly removes rectangle and destroys it */
void plane_remove_rectangle(plane_t *plane, int index) {
    if (!plane->rects[index]) return;
    free(plane->rects[index]);
    plane->rects[index] = NULL;
    plane->rect_alive--;
}

/* returns index of FIRST MATCH which is at least double mindist close */
int plane_get_proximate_rectangle(plane_t *plane, int index, double mindist, enum size_diff_e type) {
    int i, max;
    rectangle_t *rect = plane->rects[index];
    max = plane->rect_max;
    for (i = 0; i < max; i++) {
        rectangle_t *curr = plane->rects[i];
        if (!curr) continue;
        double dist = rectangle_distance(rect, curr);
        if (dist <= mindist) {
            if (curr == rect) continue;
            double cs = rectangle_size(curr),
                   rs = rectangle_size(rect);
            /*curr is prey*/
            if (rs / cs > SIZE_DIFF_TRESHOLD) {
                /*check if it is what we need*/
                if (type != sd_prey) continue;
            }
            /*curr is hunter*/
            if (cs / rs > SIZE_DIFF_TRESHOLD) {
                if (type != sd_hunter) continue;
            }
            return i;
        };
    }
    return -1;
}

double rectangle_distance(rectangle_t *left, rectangle_t *right) {
    double lcx, lcy, rcx, rcy, dcx, dcy;
    lcx = (left->x  + left->width)  / 2;
    lcy = (left->y  + left->height) / 2;

    rcx = (right->x + right->width) / 2;
    rcy = (right->y + right->height)/ 2;
    
    dcx = (lcx - rcx);
    dcy = (lcy - rcy);

    return sqrt(dcx*dcx + dcy*dcy);
}

int plane_is_rect_alive(plane_t *plane, int index) {
    if (index < 0) return false;
    if (index > plane->rect_max) return false;
    return plane->rects[index] != NULL;
}
