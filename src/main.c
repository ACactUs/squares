#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include "render.h"
#include "logic.h"

int main(int argc, char **argv){
    render_state_t *state = render_init();

    render_greeting(state);

    render_exit(state);
    return 0;
}
