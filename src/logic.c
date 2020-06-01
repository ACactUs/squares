#include "logic.h"
#include <math.h>
#include <stdlib.h>

void rectangle_default_traits(rectangle_t *rect){
    rect->traits[ti_mspeed          ] = DEFAULT_MSPEED;
    rect->traits[ti_accel           ] = DEFAULT_ACCEL;
    rect->traits[ti_nostim_secs     ] = DEFAULT_NOSTIM_SECS;
    rect->traits[ti_mrandom_delay   ] = DEFAULT_MRANDOM_DELAY;
    rect->traits[ti_avoid_dist      ] = DEFAULT_AVOID_DIST;
    rect->traits[ti_avoid_speed     ] = DEFAULT_AVOID_SPEED;
    rect->traits[ti_pursue_dist     ] = DEFAULT_PURSUE_DIST;
    rect->traits[ti_pursue_speed    ] = DEFAULT_PURSUE_SPEED;
    rect->traits[ti_food_dist       ] = DEFAULT_FOOD_DIST;
    rect->traits[ti_food_speed      ] = DEFAULT_FOOD_SPEED;
}

rectangle_t *rectangle_create() {
    rectangle_t *rect = (rectangle_t*)calloc(1, sizeof(rectangle_t));

    rect->actions.nostim_o  = RANDOM_NO_STIM; 
    rect->actions.big_o     = RANDOM_BIG; 
    rect->actions.food_o    = RANDOM_FOOD; 
    rect->actions.prey_o    = RANDOM_PREY; 
    
    rectangle_default_traits(rect);
    return rect;
}

void rectangle_destroy(rectangle_t *rect) {
    if (rect->name) free(rect->name);
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

void rectangle_collision_fight(plane_t *plane, size_t left, size_t right) {
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
    size_t looser = (winner == lrect) ? right : left;

    plane_remove_rectangle(plane, looser);

    return;
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
        default:
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
        default:
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
        default:
            exit(1);
    }
}

void action_prey(plane_t *plane, size_t index) {
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
        default:
            exit (1);
    }
}


void rectangle_accelerate(rectangle_t *rect, double speed, double angle) {
    double time_needed = speed / rect->traits[ti_accel]; /* time in seconds to accelerate */

    //TODO handle negative angles
    while (angle > 2*M_PI) angle -= M_PI;
    double x=0, y=0;
    int dl  = angle >= 0        && angle <= M_PI/2,
        ul  = angle > M_PI/2    && angle <= M_PI,
        ur  = angle > M_PI      && angle <= M_PI*1.5,
        dr  = angle > M_PI*1.5  && angle <= M_PI*2;
    /* calculate speed projections on x and y axis */
    if (dl) {
        x = -speed * sin(angle);
        y =  speed * cos(angle);
    } else
    if (ul) {
        angle -= M_PI/2;
        x = -speed * cos(angle);
        y = -speed * sin(angle);
    } else
    if (ur) {
        angle -= M_PI;
        x =  speed * sin(angle);
        y = -speed * cos(angle);
    } else
    if (dr) {
        angle -= M_PI*1.5;
        x = speed * sin(angle);
        y = speed * cos(angle);
    } else exit(1);
    
    double dx = (x * TICK_NSEC) / NSEC_IN_SEC;
    double dy = (y * TICK_NSEC) / NSEC_IN_SEC;

    /* check if speed projection sigh and check if acceleration will be completed at this iteration */
    if (x >= 0) rect->xspeed = (rect->xspeed + dx >= x) ? x : rect->xspeed + x;
        else    rect->xspeed = (rect->xspeed + dx <= x) ? x : rect->xspeed + x;
    if (y >= 0) rect->yspeed = (rect->yspeed + dy >= y) ? y : rect->yspeed + y;
        else    rect->yspeed = (rect->yspeed + dy <= y) ? y : rect->yspeed + y;
    //FIXME no energy losses
}

void rectangle_TESTMOVE     (plane_t *plane, size_t index) { 
    //FIXME redirecting to move_random
    rectangle_move_random(plane, index);
    return;
}

void rectangle_hybernate    (plane_t *plane, size_t index) { rectangle_TESTMOVE(plane, index); }

void rectangle_move_random  (plane_t *plane, size_t index) { 
    rectangle_t *rect = plane->rects[index];
    //FIXME random angle 
    double angle = rect->angle;
    if (rect->secs_timer >= rect->traits[ti_mrandom_delay]) {
        angle = ((double)rand() / RAND_MAX) * 2 * M_PI;
        rect->secs_timer = 0;
        rect->angle = angle;
    }

    rectangle_accelerate(rect, 5, angle);
}

void rectangle_move_lrandom (plane_t *plane, size_t index) { rectangle_TESTMOVE(plane, index); }
void rectangle_move_avoid   (plane_t *plane, size_t index) { rectangle_TESTMOVE(plane, index); }
void rectangle_move_seek    (plane_t *plane, size_t index) { rectangle_TESTMOVE(plane, index); }

void timer_wait(struct timespec *start, long long nsecs) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    long long nsec_elapsed = (ts.tv_sec - start->tv_sec) * NSEC_IN_SEC
                                    + ts.tv_nsec - start->tv_nsec;

    if (nsec_elapsed < nsecs) {
        //clock_gettime(CLOCK_MONOTONIC, &start);
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
    clock_gettime(CLOCK_MONOTONIC, start);
}

void timer_waitreset(struct timespec *start, long long nsecs) {
    timer_wait(start, nsecs);
    timer_reset(start);
}

int timer_check(struct timespec *start, long long nsecs) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    long long nsec_elapsed = (ts.tv_sec - start->tv_sec) * NSEC_IN_SEC
                                    + ts.tv_nsec - start->tv_nsec;


    return (nsec_elapsed < nsecs);
}

void frame_simulate(plane_t *plane) {
    timer_wait(&plane->ts_curr, TICK_NSEC);
    size_t max = plane->rect_max;
    size_t i;
    for (i = 0; i < max; i++) {
        if (!plane->rects[i]) continue;
        rectangle_act(plane, i);
    }
    timer_reset(&plane->ts_curr);
}


enum actions rectangle_action_get(plane_t *plane, size_t index) {
    rectangle_t *rect = plane->rects[index];
    int bigger_exists = false,
        target_exists = false;
    plane_get_proximate_rectangle(plane, index, rect->traits[ti_avoid_dist], sd_hunter, &bigger_exists);
    plane_get_proximate_rectangle(plane, index, rect->traits[ti_pursue_speed], sd_prey, &target_exists);
    //TODO actions order trait
    if (bigger_exists) return a_big;
    if (target_exists) return a_prey;
    //TODO food
    
    return a_no_stim;
}

void rectangle_act(plane_t *plane, size_t index) {
    enum actions action = rectangle_action_get(plane, index);
    rectangle_t *rect = plane->rects[index];
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

    switch (action) {
        case a_no_stim:
            action_nostim(plane, index);
            break;
        case a_food:
            action_food(plane, index);
            break;
        case a_big:
            action_big(plane, index);
            break;
        case a_prey:
            action_prey(plane, index);
            break;
    }
    rectangle_simulate(plane, index);
    rect->prev_action = action;
}

int rectangle_borders_resolve(plane_t *plane, size_t index) {
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
void rectangle_collision_resolve(plane_t *plane, size_t index) {
    if (!plane->rects[index]) return; /* rectangle was killed, exit recursion */
    int flag_collided;
    size_t col_index = plane_check_collisions(plane, index, &flag_collided);
    if (!flag_collided) { /* if no rect collision check borders */
        if(rectangle_borders_resolve(plane, index)) /* if changed go deeper */
            rectangle_collision_resolve(plane, index);
        return; /* no rect collision and no border collision exit recursion 
                 * or terminate exiting recursion */
    }

    /* rect collision happened, resolve by tunneling distance then go deeper */
    /* go deeper */
    //FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME FIXME 
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

void rectangle_simulate(plane_t *plane, size_t index) {
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

void  plane_init(plane_t *plane, rectangle_t **rects, size_t recs_size) {
    plane->rects = calloc(sizeof(rectangle_t*), recs_size);
    plane->rect_max = recs_size;
    timer_reset(&plane->ts_init);
    timer_reset(&plane->ts_curr);
    if (!rects){
        /* no rects given, therefore create them accordingly to config globals
         * TODO kill intersecting ones*/
        size_t i;
        for (i = 0; i < recs_size; i++) {
            rectangle_t *rect = plane->rects[i] = rectangle_create();
            _rectangle_init_randomly(plane, rect);
            size_t j = 0;
            int flag_collided = false;
            while (j++ < RECTANGLE_INIT_MAX_RETRIES) { 
                plane_check_collisions(plane, i, &flag_collided);
                if (!flag_collided) break;
                _rectangle_init_randomly(plane, rect);        
            }
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
    if (!plane) return;
    size_t i, max;
    max = plane->rect_max;
    rectangle_t **rects = plane->rects;
    for(i = 0; i < max; i++) {
        if (!rects[i]) continue;
        rectangle_destroy(rects[i]);
    }
    free(rects);
    free(plane);
}


/* Checks for collisions for rectangle of given index 
 * WARN assumes that index is valid 
 * this function does not check for NULL rectangles at index 
 * return size_t index of collision rectangle
 * if collision happed set flag to true if not, false and return 0 */
size_t plane_check_collisions(plane_t *plane, size_t index, int *flag_collided) {
    size_t i;
    rectangle_t *rect = plane->rects[index];
    for (i = 0; i < plane->rect_max; i++) {
        rectangle_t *curr = plane->rects[i];
        if (!curr) continue;
        if (rectangle_check_collision(curr, rect) && rect != curr) {
            *flag_collided = true;
            return i;
        }
    }
    *flag_collided = false;
    return 0;
}

/*WARN function assumes that left and right exist and were initialized correctly*/
int rectangle_check_collision(rectangle_t *l, rectangle_t *r) {
    /*TODO return enum of collision direction instead*/
    int iu, id, il, ir, iv, ih;
    iu = l->y >= r->y && l->y <= r->y + r->height;
    id = l->y + l->height >= r->y && l->y + l->height <= r->y + r->height;
    il = l->x >= r->x && l->x <= r->x + r->width;
    ir = l->x + l->width >= r->x && l->x + l->width <= r->x + r->width;

    iv = (id) || (l->y <= r->y && l->y + l->height >= r->y + r->height) || (iu);
    ih = (il) || (l->x <= r->x && l->x + l->width >= r->x + r->width)   || (ir);

    return (iv && ih);

    /*
    if (right->x > left->x && right->x < left->x + left->width){
        if (right->y > left->y && right->y < left->y + left->height)
            return 1;
        if (right->y + right->height > left->y && right->y + right->height < left->y + left->height)
            return 1;
    }
    if (right->x + right->width > left->x && right->x + right->width < left->x + left->width) {
        if(right->y > left->y && right->y < left->y + left->height)
            return 1;
        if(right->y + right->height > left->y && right->y + right->height < left->y + left->height)
            return 1;
    }
    return 0;
    */
}


/* properly removes rectangle and destroys it */
void plane_remove_rectangle(plane_t *plane, size_t index) {
    free(plane->rects[index]);
    plane->rects[index] = NULL;
}
size_t plane_get_proximate_rectangle(plane_t *plane, size_t index, double mindist, enum size_diff_e type, int *flagExists) {
    size_t i, max;
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
            *flagExists = true;
            return i;
        };
    }
    *flagExists = false;
    return 0;
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

int plane_is_rect_alive(plane_t *plane, size_t index) {
    if (index < 0) return false;
    if (index > plane->rect_max) return false;
    return plane->rects[index] != NULL;
}
