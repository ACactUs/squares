#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include "render.h"
#include "logic.h"

int main(int argc, char **argv){
    srand((unsigned int)time(NULL));
    render_state_t *state = render_init();

    render_greeting(state);
    plane_t *plane = plane_create(state->maxx, state->maxy);
    plane_init(plane, NULL, 11);

    render_load(state, plane);
    
    while (true) {
        render_frame(state);
        wgetch(state->status);
        render_status(state, "OK: Frame rendered_successfully");
    }

    return 0;
}
