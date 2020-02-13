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

    render_load(state, plane);
    
    while (true) {
        frame_simulate(plane);
        render_frame(state);
    }

    return 0;
}
