#include "window.h"
#include <stdlib.h>
#include <ncurses.h>

int msg_y = 0;
WINDOW *window = NULL;

void screen_close() {
    delwin(window);
    endwin();
}

void screen_init() {
    initscr();
    cbreak();
    noecho();
    int max_x = 0;
    int max_y = 0;
    getmaxyx(stdscr, max_y, max_x);
    window = newwin(3, max_x, max_y - 3, 0);
    box(window, 0, 0);
    refresh();
    wrefresh(window);
}

char *check_msg_input() {
    wclear(window);
    box(window, 0, 0);
    wrefresh(window);

    char *msg = malloc(100);

    echo();
    mvwscanw(window, 1, 1, "%99[^\n]", msg);
    noecho();

    return msg;
}

void print_msg(char *usrname, char *msg) {
    mvprintw(msg_y, 1, "%s: %s", usrname, msg);
    refresh();
    wrefresh(window);
    msg_y++;
}

void print_announcement(char *msg) {
    mvprintw(msg_y, 1, "%s", msg);
    refresh();
    wrefresh(window);
    msg_y++;
}
