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
    plane_t *plane = plane_create(state->maxx, state->maxy);
    plane_init(plane, NULL, 12);

    render_load(state, plane);
    
    while (true) {
        render_frame(state);
        frame_simulate(plane);
    }

    return 0;
}
