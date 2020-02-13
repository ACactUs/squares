#include <ncurses.h>
#include <time.h>
#include "logic.h"


#define RENDER_NSEC  16666666LL             /*60fps*/

enum zoom       { zo_none };
enum color_pairs{ cp_wb=1, cp_bw };

typedef struct {
   enum zoom zoom;          /*selected zoom level*/
   int  maxx, maxy;         /*terminal window size*/
   int canv_maxx, canv_maxy;/*size of window where rectangles will be rendered*/
   plane_t *plane;      
   WINDOW *status;          /*statusbar window*/
   WINDOW *canvas;          /*window which holds rectangles*/
   WINDOW **rect_wins;/* windows are used to render rectangles so
                      * win_rect_alive is an array of windows or NULLs, where
                      * living rectangles have same index plane_t->rect_array
                      * as their windows in render_state_t->rect_wins 
                      * if rectangle is dead then corresponding index in plane_t and
                      * in render_state_t point to NULL element*/
   struct timespec ts_last;
   struct timespec ts_init;
} render_state_t;

render_state_t *render_init(); /*done*/
void render_exit(render_state_t *state); /*done*/

void render_load(render_state_t *state, plane_t *plane); /*done*/
void render_unload(render_state_t *state, plane_t *plane); /*done*/

void render_greeting(render_state_t *state); /*done*/

void zoom_set(plane_t *plane); 

void render_frame(render_state_t *state); /*done*/

void render_rectangle(render_state_t *state, size_t index); /*done*/

void render_status(render_state_t *state, char *message); /*done*/

void render_clear_status(render_state_t *state); /*done*/
