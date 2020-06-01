#include "render.h"
#include "logic.h"
#include <stdio.h>
#include <time.h>
#include <curses.h>
#include <string.h>

void render_greeting(render_state_t *state) {
    const size_t msize = 512;
    char message[msize];
    snprintf(message, msize, 
            "Press h to get help and see options\n\nScreen width %d, height %d\nPlane width %lf, height %lf\nPlane has %zu rectangles", 
            state->canv_maxx, state->maxy, 
            state->plane->xsize, state->plane->ysize, 
            state->plane->rect_max);
    render_status(state, "Status initialized..... OK");
    box(state->canvas, 0, 0);
    render_popup_getch(state, message);
    wrefresh(state->canvas);
}

void render_frame(render_state_t *state){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    long long nsec_elapsed = (ts.tv_sec - state->ts_last_render.tv_sec) * 1000000000LL
                                    + ts.tv_nsec - state->ts_last_render.tv_nsec;
    if (nsec_elapsed < RENDER_NSEC) return;

    size_t rect_max = state->plane->rect_max;
    size_t i;
    box(state->canvas, 0, 0);
    wnoutrefresh(state->canvas);
    for (i = 0; i < rect_max; i++) {
        rectangle_t *rect = state->plane->rects[i];
        if (!rect) continue;
        render_rectangle(state, i);
    }

    

    clock_gettime(CLOCK_MONOTONIC, &state->ts_last_render);
    doupdate();
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

    //FIXME dont update for each rect
    wclear(win);
    //wrefresh(win);
    
    int lines, cols;
    lines = (int)(rect->height); 
    cols  = (int)(rect->width);
    if (lines < 1) lines = 1;
    if (cols  < 1) cols  = 1;

    int mresp = mvwin(win, (int)rect->y, (int)rect->x);
    int rresp = wresize(win, lines, cols);

    size_t m_len = 5;
    char message[m_len];
    snprintf(message, m_len, "%zu", index);
    mvwprintw(win, 1, 1, message);


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
    wnoutrefresh(win);
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
    if (!state) return;
    if (state->plane) {
        size_t wins = state->plane->rect_max;
        for (i = 0; i < wins; i++) {
            WINDOW *win = state->rect_wins[i];
            if (!win) continue;
            delwin(win);
            state->rect_wins[i] = NULL;
        }
    }
    free(state->rect_wins);
    state->plane = NULL;
}

render_state_t *render_init() {
    render_state_t *state = calloc(sizeof(render_state_t), 1);
    initscr();
    cbreak();
    
    /* TODO support colorless terms */
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
    clock_gettime(CLOCK_MONOTONIC, &state->ts_last_render);
    wattron(state->status, A_REVERSE);
    nodelay(state->status, true);
    noecho();
    refresh();
    return state;
}

void render_status(render_state_t *state, char *message) { 
    render_clear_status(state);
    mvwprintw(state->status, 0, 0, message);
    
    //FIXME
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
    exit(0);
}

/* returns -1 on fail */
int calculate_height(char *message, int width) {
    const int   max_rets = 32;

    int i;
    char *start_ptr = message;  /*& of char after last \n*/
    char *nl_ptr;               /* current \n or NULL*/
    int lens[max_rets];
    int newlines = 0;
    for (i = 0; i < max_rets; i++) {
        newlines++;
        nl_ptr = strchr(start_ptr, '\n');
        /* calculate max width */
        if (nl_ptr == NULL) {
            size_t len = strlen(start_ptr);
            lens[i] = (int)len;
            break;
        } else
        if (*nl_ptr == '\n') {
            size_t len = (size_t)(start_ptr - nl_ptr);
            lens[i] = (int)len;
            start_ptr = nl_ptr + 1;
            continue;
        } 
    }

    if (i == max_rets) return -1;
    
    /*calculate height*/
    int height = newlines;
    for (i = 0; i < newlines; i++) {
        //FIXME "left operand has garbage value"
        int carry = lens[i] / width;
        height += carry;
        if (carry && lens[i] % width != 0) height++;
    }

    return height;
}

/*FIXME last line is not being printed*/
int render_popup_getch(render_state_t *state, char *message) {
    int width  = (int)(state->maxx * 0.6);
    int height = calculate_height(message, width);

    if (height == -1) return 0;

    int x0 = (state->maxx - width)/2;
    int y0 = (state->maxy - height)/2;

    WINDOW *box = newwin(height+2, width+2, y0-1, x0-1);
    box(box, 0, 0);
    wrefresh(box);

    WINDOW *popup = newwin(height, width, y0, x0);
    wprintw(popup, message);
    wrefresh(popup);

    nodelay(popup, false);
    int key = wgetch(popup);

    delwin(box);
    delwin(popup);

    return key;
}

/* errno is set by strtol if it encounters error
 * enables echo, does not change nodelay*/
long input_int(WINDOW *win) {
    int was_nodelay = is_nodelay(win);
    nodelay(win, false);
    echo();
    char input[16];
    //TODO check if it does not cause overflow
    wgetnstr(win, input, sizeof(input));
    long num = strtol(input, NULL, 0);

    if (was_nodelay) nodelay(win, true);
    if (!input[0] && !num) errno = EIO;
    return num;
}

/* WARN string must be freed after use */
char *input_string(render_state_t *state, char *prompt) {
    WINDOW *win = state->status;
    int was_nodelay = is_nodelay(win);
    nodelay(win, false);
    echo();
    int i_size = 128;
    char *input = calloc(sizeof(char), (size_t)i_size);
    mvwprintw(win, 0, 0, prompt);
    wgetnstr(win, input, i_size);

    if (was_nodelay) nodelay(win, true);
    return input;
}

/* returns 0 on success */
int input_2doubles(render_state_t *state, char *message, double *a, double *b) {
    float x, y;
    char *input = input_string(state, message);
    int nscaned = sscanf(input, "%f %f", &x, &y);
    free(input);

    if (nscaned != 2) {
        render_status(state, "Input failed");
        return -1;
    }

    *a = x;
    *b = y;

    return 0;
}

void ckey_space(render_state_t *state) {
    /* FIXME messes up in-game time representation
     * pause should be defined in logic.c and used inside simulate func */
    nodelay(state->status, false);
    render_status(state, "Game is paused, press any key to unpause...");
    wgetch(state->status);
    nodelay(state->status, true);
    render_status(state, "Unpaused...");
}

void ckey_k(render_state_t *state) {
    render_status(state, "Kill index: ");
    size_t target = (size_t)input_int(state->status);
    noecho();
    if (!errno) {
        if (plane_is_rect_alive(state->plane, target)) {
            plane_remove_rectangle(state->plane, target);
            render_status(state, "Removed!");
        } else {
            render_status(state, "Rectangle does not exist");
        }
    } else {
        errno = 0;
        render_status(state, "Not a number");
    }
}

void ckey_p(render_state_t *state) {
    const int max_size = 512;
    char message[max_size];

    render_status(state, "Print index: ");
    size_t target = (size_t)input_int(state->status);
    noecho();
    if (!errno) {
        if (plane_is_rect_alive(state->plane, target)) {
            render_status(state, "Printing");
            rectangle_t *rect = state->plane->rects[target];
            snprintf(message, max_size, 
                    "I: %zu, N: %s,\na: %f, y: %f, x: %f, h: %f, w: %f\nenrg: %f",
                    target, rect->name, 
                    rect->angle, rect->y, rect->x, rect->height, rect->width,
                    rect->energy);
            render_popup_getch(state, message);
        } else {
            render_status(state, "Rectangle does not exist");
        }
    } else {
        errno = 0;
        render_status(state, "Not a number");
    }
}

/* move left corner */
void ckey_e1(render_state_t *state, size_t target) {
    rectangle_t *rect = state->plane->rects[target];
    char msg[64] = {0};
    snprintf(msg, 64, "(x=%f, y=%f)Enter new [x y]: ", rect->x, rect->y);

    double x, y;
    int ret = input_2doubles(state, msg, &x, &y);

    if (ret != 0) return;

    if (x < 0 || y < 0 || 
        x + rect->width > state->plane->xsize || 
        y + rect->height > state->plane->ysize
    ) {
        render_status(state, "Invalid coordinates");
    } else {
        rect->x = x;
        rect->y = y;
    }
}

/* resize */
void ckey_e2(render_state_t *state, size_t target) {
}

void ckey_emenu(render_state_t *state, size_t target, int key) {
    switch (key) {
        case '1':
            ckey_e1(state, target);
            break;
        case '2':
        case '3':
        case '4':
        case '5':
            render_popup_getch(state, "Not implemented");
            break;
    }
}

void ckey_e(render_state_t *state) {
    const int max_size = 512;
    char message[max_size];

    render_status(state, "Edit index: ");
    size_t target = (size_t)input_int(state->status);
    noecho();
    if (!errno) {
        if (plane_is_rect_alive(state->plane, target)) {
            render_status(state, "Editing");
            snprintf(message, max_size, 
                    "Press key to edit\n-------\n1: left corner [x, y]\n2: [width, height]\n3: [energy]\n4: actions\n5: traits");
            int key = render_popup_getch(state, message);
            render_frame(state);
            ckey_emenu(state, target, key);
        } else {
            render_status(state, "Rectangle does not exist");
        }
    } else {
        errno = 0;
        render_status(state, "Not a number");
    }
}

int ckey_r(render_state_t *state) {
    int key = render_popup_getch(state, "Restart? [y,r/N]");
    if (key == 'y' || key == 'r') {
        plane_t *p = state->plane;
        render_unload(state);
        plane_destroy(p);

        /* TODO ask user to fill fields*/
        p = plane_create(state->canv_maxx+1, state->canv_maxy+1);
        plane_init(p, NULL, 10);
        render_load(state, p);
        return false;
    }
    return true;
}

int control_cycle(render_state_t *state) {
    /* check clock */
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    long long nsec_elapsed = (ts.tv_sec - state->ts_last_input.tv_sec) * 1000000000LL
                                    + ts.tv_nsec - state->ts_last_input.tv_nsec;
    if (nsec_elapsed < RENDER_NSEC) return 0;
    
    /* read key TODO, implement input window*/

    int key = wgetch(state->status);
    int is_event = true;
    switch (key) {
        /* no input this time*/
        case ERR:
            is_event = false;
            break;
        /* space -> pause game until pressed again*/
        case ' ':
            ckey_space(state);
            break;
        case 'k':
            ckey_k(state);
            break;
        case 'h':
            {
                char *message = "HELP\n------------\n[h]: see help\n[e]: edit rectangle\n[k]: kill rectangle\n[r]: restart\n[q]: quit :(\nSPACE: pause game";
                render_popup_getch(state, message);
            }
            break;
        case 'p':
            ckey_p(state);
            break;
        case 'e':
            ckey_e(state);
            break;
        case 'q':
            {
                int key = render_popup_getch(state, "Exit? [y,q/N]");
                if (key == 'y' || key == 'q') render_exit(state);
            }
            break;
        case 'r':
            {
                int is_restart = ckey_r(state);
                if (is_restart) return false;
            }
            break;

        /* TODO implement: 
         * e - edit rectangle interactively (table)
         * m - move rectangle interactively (redraw) 
         * r - restart */

        /*TODO define broad message box*/

        /* valid unused key*/
        default:
            break;
    }
    
    /* reset clock */
    clock_gettime(CLOCK_MONOTONIC, &state->ts_last_input);

    return is_event;
}

