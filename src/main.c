#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include "render.h"
#include "logic.h"

int main(int argc, char **argv){
    render_state_t *state = render_init();

    render_greeting(state);
    plane_t *plane = plane_create(40, 40);
    plane_init(plane, NULL, 1);

    render_load(state, plane);
    
    while (true) {
        render_frame(state);
        wgetch(state->status);
    }

    return 0;
}
