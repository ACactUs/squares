#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>

int main(int argc, char **argv){
    printf("program started\n");
    sleep(1);
    initscr();
    cbreak();
    noecho();
    printw("curses");
    refresh();
    getch();
    endwin();
    return 0;
}
