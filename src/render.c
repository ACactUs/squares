#include "render.h"
#include "logic.h"
#include <time.h>
#include <curses.h>
#include <string.h>

void render_greeting(render_state_t *state) {
    WINDOW *greetings = newwin(5, 40, state->maxy/2 - 3, state->maxx/2 - 20);
    box(greetings, 0, 0);
    mvwprintw(greetings, 2, 2, "Please, do not resize the terminal");
    render_status(state, "Statusbar initialized..... OK");
    wgetch(greetings);
    wclear(greetings);
    wrefresh(greetings);
    delwin(greetings);
    box(state->canvas, 0, 0);
    wrefresh(state->canvas);
}

void render_frame(render_state_t *state){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    long long nsec_elapsed = (ts.tv_sec - state->ts_last.tv_sec) * 1000000000LL
                                    + ts.tv_nsec - state->ts_last.tv_nsec;
    if (nsec_elapsed < RENDER_NSEC) return;

    size_t rect_max = state->plane->rect_max;
    size_t i;
    box(state->canvas, 0, 0);
    wrefresh(state->canvas);
    for (i = 0; i < rect_max; i++) {
        rectangle_t *rect = state->plane->rects[i];
        if (!rect) continue;
        render_rectangle(state, i);
    }
    clock_gettime(CLOCK_MONOTONIC, &state->ts_last);
}

void render_rectangle(render_state_t *state, size_t index) {
    WINDOW *win = state->rect_wins[index];
    if (index >= state->plane->rect_max) {
        render_status(state, "ERR: Index too big at render_rectangle!... press any key");
        wgetch(state->status);
        return;
    }

    rectangle_t *rect = state->plane->rects[index];

    if (!rect) {
        render_status(state, "ERR: Rect does not exist at render_rectangle... press any key");
        wgetch(state->status);
        return;
    }

    if (!win) {
        render_status(state, "ERR: Rect window for this index does not exist... press any key");
        wgetch(state->status);
        return;
    }

    wclear(win);
    wrefresh(win);
    
    int lines, cols;
    lines = (int)(rect->height); 
    cols  = (int)(rect->width);
    if (lines < 1) lines = 1;
    if (cols  < 1) cols  = 1;

    int mresp = mvwin(win, (int)rect->y, (int)rect->x);
    int rresp = wresize(win, lines, cols);


    /* resize rect win to (1,1) and retry */
    if ((ERR == mresp) | (ERR == rresp)) {
        wresize(win, 1, 1);
        mresp = mvwin(win, (int)rect->y, (int)rect->x);
        rresp = wresize(win, lines, cols);
        if ((ERR == mresp) | (ERR == rresp)) {
            render_status(state, "ERR: could not move window, render canceled... press any key");
            wgetch(state->status);
            return;
        }

        //resize 1x1 TODO
        //move
        //resize h*w
    }
    
    box(win, 0, 0);
    wrefresh(win);
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

void render_unload(render_state_t *state) {
    /* close every non-NULL window*/
    size_t i;
    size_t wins = state->plane->rect_max;
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
    state->canv_maxx = state->maxx;
    state->canv_maxy = state->maxy - 1;
    state->status = newwin(1, state->maxx, state->maxy-1, 0);
    state->plane  = NULL;
    state->canvas = newwin(state->canv_maxy, state->maxx, 0, 0);
    clock_gettime(CLOCK_MONOTONIC, &state->ts_init);
    clock_gettime(CLOCK_MONOTONIC, &state->ts_last);
    wattron(state->status, A_REVERSE);
    noecho();
    refresh();
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
    plane_destroy(state->plane);
    render_unload(state);
    free(state);
    endwin();
}
