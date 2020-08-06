#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include "render.h"
#include "logic.h"

int main(){
    srand((unsigned int)time(NULL));
    render_init();

    plane_t *plane = plane_create( (rstate->canv_maxx+1) / rstate->char_htw, 
        rstate->canv_maxy+1);
    plane_populate_randomly(plane, 10);
    render_load(plane);
    render_greeting();
    time_begin();
    
    while (true) {
        time_start_logic();
        control_cycle();
        frame_simulate(rstate->plane);
        render_frame();
        time_next();
    }

    /* TODO write user interface and exit/reload plane on wgetch 
    render_exit();
    */
    return 0;
}
