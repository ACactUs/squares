#include <ncurses.h>
#include <time.h>
#include "logic.h"
#include <errno.h>

/* VERY IMPORTANT: ncurses cant get char aspect ratio on a real screen,
 * it can retrieve width and height of screen in characters
 * this constant is terminal-dependent and should be set by user
 * if this is set improperly, rectangles will have inaccurate aspect ratio,
 * i.e. 10*10 rectangle wont be rendered as square
 * this only affects rendering, not physics*/
#define DEFAULT_CHAR_HTW    2.0             /*char height / width in pixel on screen*/

#define RENDER_NSEC         33333333LL      /*60fps 16666666LL; 30fps 33333333LL*/
#define INPUT_NSEC          16666666LL      /*60fps 16666666LL; 30fps 33333333LL*/

enum zoom       { zo_none };
enum color_pairs{ cp_wb=1, cp_bw };

typedef struct {
    enum zoom zoom;          /*selected zoom level*/
    int  maxx, maxy;         /*terminal window size*/
    double char_htw;         /*terminal width to height ration*/
    int canv_maxx, canv_maxy;/*size of window where rectangles will be rendered*/
    plane_t *plane;      
    WINDOW *status;          /*statusbar window*/
    WINDOW *canvas;          /*window which holds rectangles*/
    WINDOW **rect_wins;  
    /* windows are used to render rectangles so
     * win_rect_alive is an array of windows or NULLs, where
     * living rectangles have same index plane_t->rect_array                                
     * as their windows in render_state_t->rect_wins                                        
     * if rectangle is dead then corresponding index in plane_t and                         
     * in render_state_t point to NULL element*/                                            
    struct timespec ts_last_render;
    struct timespec ts_last_input;
    struct timespec ts_init;
} render_state_t;

struct popup_state {
    WINDOW *box;
    WINDOW *popup;
};

extern render_state_t *rstate;


/********************************
 *      RENDER FUNCTIONS        *
 ********************************/

void render_init(); /*done*/
void render_exit(); /*done*/

void render_load(plane_t *plane); /*done*/
void render_unload(); /*done*/

void render_greeting(); /*done*/

void zoom_set(plane_t *plane); 

void render_frame(); /*done*/

void render_rec(int index); /*done*/

void render_status(char *message); /*done*/

void render_clear_status(); /*done*/

int render_popup_getch(char *message);

/******************************
 * USER INTERACTION FUNCTIONS *
 ******************************/

/* all user input is processed here
 * depends on ts_last_input timer
 * input freq is defined by INPUT_NSEC in header 
 * this function should only handle user input and then call 
 * functions from logic.h, to modify plane_t state */
int control_cycle();
