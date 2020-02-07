#include "render.h"
#include <curses.h>
#include <string.h>

void render_greeting(render_state_t *state) {
    WINDOW *greetings = newwin(5, 40, state->maxy/2 - 3, state->maxx/2 - 20);
    box(greetings, 0, 0);
    mvwprintw(greetings, 2, 2, "Please, do not resize the terminal");
    render_status(state, "Statusbar initialized..... OK");
    wgetch(greetings);
    delwin(greetings);
}

render_state_t *render_init() {
    render_state_t *state = calloc(sizeof(render_state_t), 1);
    initscr();
    cbreak();

    if (!has_colors() || !can_change_color() ) {
        fprintf(stderr, "Sorry, your terminal does not support color changing\n");
        render_exit(state);
        exit(1);
    }

    /*TODO init colors */
    start_color();
    use_default_colors();
    //init_pair(cp_bw, -1, COLOR_WHITE);

    getmaxyx(stdscr, state->maxy, state->maxx);
    state->zoom = zo_none;
    
    state->status = newwin(1, state->maxx, state->maxy-1, 0);
    
    wattron(state->status, A_REVERSE);

    noecho();
    return state;
}

void render_status(render_state_t *state, char *message) { 
    render_clear_status(state);
    mvwprintw(state->status, 0, 0, message);
    
    wrefresh(state->status);
}

void render_clear_status(render_state_t *state) {
    wclear(state->status);
    wrefresh(state->status);
}

void render_exit(render_state_t *state) {
    free(state);
    endwin();
}
