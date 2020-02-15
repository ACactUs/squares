#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include "render.h"
#include "logic.h"

int main(){
    srand((unsigned int)time(NULL));
    render_state_t *state = render_init();

    render_greeting(state);
    plane_t *plane = plane_create(state->canv_maxx, state->canv_maxy);
    plane_init(plane, NULL, 8);

    /* for test only */
    int i;
    for (i = 0; i < 8; i++) {
        plane->rects[i]->xspeed = 3;
        plane->rects[i]->yspeed = 3;
    }

    render_load(state, plane);
    
    while (true) {
        frame_simulate(plane);
        render_frame(state);
    }

    /* TODO write user interface and exit/reload plane on wgetch 
    render_exit(state);
    */
    return 0;
}
