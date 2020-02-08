#include "render.h"
#include "logic.h"
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


void render_frame(render_state_t *state){
    //TODO
    size_t rect_max = state->plane->rect_max;
    size_t i;
    for (i = 0; i < rect_max; i++) {
        rectangle_t *rect = state->plane->rects[i];
        if (!rect) continue;
    }
}

void render_rectangle(render_state_t *state, size_t rect_num) {
    //TODO
    render_status(state, "Cant render rectangle yet");
    return; /*DEAD CODE FOLLOWS!*/

    if (rect_num >= state->plane->rect_max) {
        render_status(state, "ERR: Index too big at render_rectangle!... press any key");
        wgetch(state->status);
    }

    rectangle_t *rect = state->plane->rects[rect_num];
    if (!rect) {
        render_status(state, "ERR: Rect does not exist at render_rectangle... press any key");
        wgetch(state->status);
    }
    /*move rect win*/
    
    /*FIXME*/
    /*resize rect win*/
    int lines, cols;
    lines = (int)(rect->height);
    cols  = (int)(rect->width);
    if (lines < 1) lines = 1;
    if (cols  < 1) cols  = 1;
    //resize 1x1
    //move
    //resize h*w
}

void render_load(render_state_t *state, plane_t *plane) {
    size_t wins = plane->rect_max;
    state->rect_wins = calloc(sizeof(WINDOW*), wins);
    state->zoom = zo_none;
    state->plane = plane;

    /* init every window*/
    size_t i;
    for (i = 0; i < wins; i++) {
        rectangle_t *rect = plane->rects[i];
        if (!rect) continue;

        int lines, cols, by, bx;
        lines = (int)rect->height;
        cols  = (int)rect->width;
        by    = (int)rect->y;
        bx    = (int)rect->x;
        if (lines < 1 ) lines = 1;
        if (cols < 1 ) lines = 1;
        state->rect_wins[i] = newwin(lines, cols, by, bx);
    }
}

void render_unload(render_state_t *state, plane_t *plane) {
    /* close every non-NULL window*/
    size_t i;
    size_t wins = plane->rect_max;
    for (i = 0; i < wins; i++) {
        WINDOW *win = state->rect_wins[i];
        if (!win) continue;
        delwin(win);
        state->rect_wins[i] = NULL;
    }
    free(state->rect_wins);
    state->plane = NULL;
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

    /*TODO fix colors */
    start_color();
    use_default_colors();
    //init_pair(cp_bw, -1, COLOR_WHITE);

    getmaxyx(stdscr, state->maxy, state->maxx);
    
    state->status = newwin(1, state->maxx, state->maxy-1, 0);
    state->plane  = NULL;
    
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
