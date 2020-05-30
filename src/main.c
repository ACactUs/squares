#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include "render.h"
#include "logic.h"

int main(){
    srand((unsigned int)time(NULL));
    render_state_t *state = render_init();

    plane_t *plane = plane_create(state->canv_maxx+1, state->canv_maxy+1);
    plane_init(plane, NULL, 10);
    render_load(state, plane);
    render_greeting(state);
    
    while (true) {
        control_cycle(state);
        frame_simulate(state->plane);
        render_frame(state);
    }

    /* TODO write user interface and exit/reload plane on wgetch 
    render_exit(state);
    */
    return 0;
}
