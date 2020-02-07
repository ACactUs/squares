#include <ncurses.h>
#include "logic.h"

enum zoom       { zo_none };
enum color_pairs{ cp_wb=1, cp_bw };

typedef struct {
   enum zoom zoom; 
   int  maxx, maxy;
   WINDOW *status;
} render_state_t;

render_state_t *render_init();

void render_greeting(render_state_t *state); /*done*/

void zoom_set(plane_t *plane);

void render_frame(plane_t *plane);

void render_exit(render_state_t *state); /*done*/

void render_status(render_state_t *state, char *message); 

void render_clear_status(render_state_t *state); /*FIXME*/
