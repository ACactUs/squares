#include "render.h"
#include "logic.h"
//#include "logic.h"
#include <stdio.h>
#include <time.h>
#include <curses.h>
#include <string.h>

render_state_t *rstate = NULL;

void render_greeting() {
    const int msize = 512;
    char message[msize];
    snprintf(message, msize, 
        "Press h to get help and see options\n\nScreen width %d, height %d\nPlane width %lf, height %lf\nPlane has %d rectangles", 
        rstate->canv_maxx, rstate->maxy, 
        rstate->plane->xsize, rstate->plane->ysize, 
        rstate->plane->rect_max);

    render_status("Status initialized..... OK");
    //box(0, 0);
    render_popup_getch(message);
    wrefresh(rstate->canvas);
}

void render_frame(){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    long long nsec_elapsed = (ts.tv_sec - rstate->ts_last_render.tv_sec) * 1000000000LL
                                    + ts.tv_nsec - rstate->ts_last_render.tv_nsec;
    if (nsec_elapsed < RENDER_NSEC) return;

    int rect_max = rstate->plane->rect_max;
    int i;
    box(rstate->canvas, 0, 0);
    wnoutrefresh(rstate->canvas);
    for (i = 0; i < rect_max; i++) {
        rectangle_t *rect = rstate->plane->rects[i];
        if (!rect) continue;
        render_rectangle(i);
    }

    clock_gettime(CLOCK_MONOTONIC, &rstate->ts_last_render);
    doupdate();
}

void render_rectangle(int index) {
    WINDOW *win = rstate->rect_wins[index];
    if (index >= rstate->plane->rect_max) {
        render_status("ERR: Index too big at render_rectangle!... press any key");
        wgetch(rstate->status);
        return;
    }

    rectangle_t *rect = rstate->plane->rects[index];

    if (!rect) {
        render_status("ERR: Rect does not exist at render_rectangle... press any key");
        wgetch(rstate->status);
        return;
    }

    if (!win) {
        render_status("ERR: Rect window for this index does not exist... press any key");
        wgetch(rstate->status);
        return;
    }

    //FIXME dont update for each rect
    werase(win);
    //wrefresh(win);
    
    int lines, cols;
    lines = (int)(rect->height); 
    cols  = (int)(rect->width);
    if (lines < 1) lines = 1;
    if (cols  < 1) cols  = 1;

    int mresp = mvwin(win, (int)rect->y, (int)rect->x);
    int rresp = wresize(win, lines, cols);

    int m_len = 5;
    char message[m_len];
    snprintf(message, (size_t)m_len, "%d", index);
    mvwprintw(win, 1, 1, message);


    /* resize rect win to (1,1) and retry */
    if ((ERR == mresp) | (ERR == rresp)) {
        wresize(win, 1, 1);
        mresp = mvwin(win, (int)rect->y, (int)rect->x);
        rresp = wresize(win, lines, cols);
        if ((ERR == mresp) | (ERR == rresp)) {
            render_status("ERR: could not move window, render canceled... press any key");
            wgetch(rstate->status);
            return;
        }

        //resize 1x1 TODO
        //move
        //resize h*w
    }
    
    box(win, 0, 0);
    wnoutrefresh(win);
}

void render_load(plane_t *plane) {
    int wins = plane->rect_max;
    rstate->rect_wins = calloc(sizeof(WINDOW*), (size_t)wins);
    rstate->zoom = zo_none;
    rstate->plane = plane;

    /* init every window*/
    int i;
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
        rstate->rect_wins[i] = newwin(lines, cols, by, bx);
    }
}

void render_unload() {
    /* close every non-NULL window*/
    int i;
    if (!rstate) return;
    if (rstate->plane) {
        int wins = rstate->plane->rect_max;
        for (i = 0; i < wins; i++) {
            WINDOW *win = rstate->rect_wins[i];
            if (!win) continue;
            delwin(win);
            rstate->rect_wins[i] = NULL;
        }
    }
    free(rstate->rect_wins);
    rstate->plane = NULL;
}

void render_init() {
    rstate = calloc(sizeof(render_state_t), 1);
    initscr();
    cbreak();
    
    /* TODO support colorless terms */
    if (!has_colors() || !can_change_color() ) {
        fprintf(stderr, "Sorry, your terminal does not support color changing\n");
        render_exit();
        
        exit(1);
    }

    /*TODO fix colors */
    start_color();
    use_default_colors();
    //init_pair(cp_bw, -1, COLOR_WHITE);

    getmaxyx(stdscr, rstate->maxy, rstate->maxx);
    rstate->canv_maxx = rstate->maxx;
    rstate->canv_maxy = rstate->maxy - 1;
    rstate->status = newwin(1, rstate->maxx, rstate->maxy-1, 0);
    rstate->plane  = NULL;
    rstate->canvas = newwin(rstate->canv_maxy, rstate->maxx, 0, 0);
    clock_gettime(CLOCK_MONOTONIC, &rstate->ts_init);
    clock_gettime(CLOCK_MONOTONIC, &rstate->ts_last_render);
    wattron(rstate->status, A_REVERSE);
    nodelay(rstate->status, true);
    noecho();
    refresh();
}

void render_status(char *message) { 
    render_clear_status();
    mvwprintw(rstate->status, 0, 0, message);
    
    //FIXME
    wrefresh(rstate->status);
}

void render_clear_status() {
    wclear(rstate->status);
    wrefresh(rstate->status);
}

void render_exit() {
    plane_destroy(rstate->plane);
    render_unload();
    free(rstate);
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
        } else if (*nl_ptr == '\n') {
            int len = (nl_ptr - start_ptr);
            lens[i] = len;
            start_ptr = nl_ptr + 1;
            continue;
        } else {
            render_exit();
            exit(1);
        }
    }

    if (i == max_rets) return -1;
    
    /*calculate height*/
    int height = newlines;
    for (i = 0; i < newlines; i++) {
        int carry = lens[i] / width;
        height += carry;
    }

    return height;
}

int render_popup_getch(char *message) {
    int width  = (int)(rstate->maxx * 0.6);
    int height = calculate_height(message, width);

    if (height == -1) return 0;

    int x0 = (rstate->maxx - width)/2;
    int y0 = (rstate->maxy - height)/2;

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

/* returns 1 on success, writes int to res address */
int input_int(int *res) {
    WINDOW *win = rstate->status;
    int was_nodelay = is_nodelay(win);
    nodelay(win, false);
    echo();

    char input[16];
    int ret = wgetnstr(win, input, sizeof(input) - 1);
    if (ret != OK) return 0;
    *res = (int)strtol(input, NULL, 0);

    if (was_nodelay) nodelay(win, true);
    if (!input[0] && !*res) return 0;
    return 1;
}

/* WARN string must be freed after use */
char *input_string(char *prompt) {
    WINDOW *win = rstate->status;
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
int input_2doubles(char *message, double *a, double *b) {
    float x, y;
    char *input = input_string(message);
    int nscaned = sscanf(input, "%f %f", &x, &y);
    free(input);

    if (nscaned != 2) {
        render_status("Input failed");
        return -1;
    }

    *a = x;
    *b = y;

    return 0;
}

int input_select_rect(char *prompt) {
    if (prompt) render_status(prompt);

    int target;
    int is_success = input_int(&target);
    noecho();

    if (is_success) {
        if (plane_is_rect_alive(rstate->plane, target)) { //FIXME target may be uninitialized
            return target;
        } else {
            render_status("Rectangle does not exist");
        }
    } else {
        render_status("Invalid index");
    }
    return -1;
}

void ckey_space() {
    nodelay(rstate->status, false);
    render_status("Game is paused, press any key to unpause...");
    wgetch(rstate->status);
    nodelay(rstate->status, true);
    render_status("Unpaused...");
}

void ckey_k() {
    int target = input_select_rect("Kill index: ");
    if (target == -1) return;

    plane_remove_rectangle(rstate->plane, target);
    render_status("Removed!");
}

void ckey_p() {
    int target = input_select_rect("Print rectangle: ");
    if (target == -1) return;

    render_status("Printing");
    rectangle_t *rect = rstate->plane->rects[target];

    char *message;
    size_t bufsize = rectangle_represent(rect, &message);
    if (bufsize == 0) return;

    render_popup_getch(message);
}

/* move left corner */
void ckey_e1(int target) {
    rectangle_t *rect = rstate->plane->rects[target];
    char msg[64] = {0};
    snprintf(msg, 64, "(x=%f, y=%f)Enter new [x y]: ", rect->x, rect->y);

    double x, y;
    int ret = input_2doubles(msg, &x, &y);

    if (ret != 0) return;

    if (x < 0 || y < 0 || 
        x + rect->width > rstate->plane->xsize || 
        y + rect->height > rstate->plane->ysize
    ) {
        render_status("Invalid coordinates");
    } else {
        rect->x = x;
        rect->y = y;
    }
}

/* resize */
void ckey_e2(int target) {
}

void ckey_emenu(int target, int key) {
    switch (key) {
        case '1':
            ckey_e1(target);
            break;
        case '2':
        case '3':
        case '4':
        case '5':
            render_popup_getch("Not implemented");
            break;
    }
}

void ckey_e() {
    int target = input_select_rect("Edit rectangle: ");
    if (target == -1) return;

    const int max_size = 512;
    char message[max_size];

    render_status("Editing");
    snprintf(message, max_size, 
            "Press key to edit\n-------\n1: left corner [x, y]\n2: [width, height]\n3: [energy]\n4: actions\n5: traits");
    int key = render_popup_getch(message);
    render_frame();
    ckey_emenu(target, key);
}

int ckey_r() {
    int key = render_popup_getch("Restart? [y,r/N]");
    if (key == 'y' || key == 'r') {
        plane_t *p = rstate->plane;
        render_unload();
        plane_destroy(p);

        /* TODO ask user to fill fields*/
        p = plane_create(rstate->canv_maxx+1, rstate->canv_maxy+1);
        plane_init(p, NULL, 10);
        render_load(p);
        return false;
    }
    return true;
}

/* FIXME messes up in-game time representation
 * pause should be defined in logic.c and used inside simulate func */
int control_cycle() {
    /* check clock */
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    long long nsec_elapsed = (ts.tv_sec - rstate->ts_last_input.tv_sec) * 1000000000LL
                                    + ts.tv_nsec - rstate->ts_last_input.tv_nsec;
    if (nsec_elapsed < RENDER_NSEC) return 0;
    
    /* read key TODO, implement input window*/

    int key = wgetch(rstate->status);
    int is_event = true;
    switch (key) {
        /* no input this time*/
        case ERR:
            is_event = false;
            break;
        /* space -> pause game until pressed again*/
        case ' ':
            ckey_space();
            break;
        case 'k':
            ckey_k();
            break;
        case 'h':
            {
                char *message = "HELP\n------------\n[h]: see help\n[e]: edit rectangle\n[k]: kill rectangle\n[r]: restart\n[q]: quit :(\nSPACE: pause game";
                render_popup_getch(message);
            }
            break;
        case 'p':
            ckey_p();
            break;
        case 'e':
            ckey_e();
            break;
        case 'q':
            {
                int key = render_popup_getch("Exit? [y,q/N]");
                if (key == 'y' || key == 'q') render_exit();
            }
            break;
        case 'r':
            {
                int is_restart = ckey_r();
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
    clock_gettime(CLOCK_MONOTONIC, &rstate->ts_last_input);

    return is_event;
}

