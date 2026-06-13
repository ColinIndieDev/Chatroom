#include "window.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

int msg_y = 1;
WINDOW *input_win = NULL;
WINDOW *chat_win = NULL;

void screen_close() {
    delwin(input_win);
    delwin(chat_win);
    endwin();
}

void screen_init(char *usrname) {
    initscr();
    cbreak();
    noecho();

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_CYAN, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_WHITE, COLOR_BLACK);
        init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    }

    int max_x = 0;
    int max_y = 0;
    getmaxyx(stdscr, max_y, max_x);

    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(0, 1, " Super Epic Private Chatroom ");
    attroff(COLOR_PAIR(1) | A_BOLD);

    char user_tag[40];
    snprintf(user_tag, sizeof(user_tag), "You: %s ", usrname);
    mvprintw(0, (int)(max_x - strlen(user_tag) - 1), user_tag);
    mvhline(1, 0, ACS_HLINE, max_x);

    chat_win = newwin(max_y - 5, max_x, 2, 0);
    box(chat_win, 0, 0);

    wattron(chat_win, COLOR_PAIR(1));
    mvwprintw(chat_win, 0, 2, "[ Messages ]");
    wattroff(chat_win, COLOR_PAIR(1));

    input_win = newwin(3, max_x, max_y - 3, 0);
    box(input_win, 0, 0);
    wattron(input_win, COLOR_PAIR(4));
    mvwprintw(input_win, 0, 2, "[ Type Message ]");
    wattroff(input_win, COLOR_PAIR(4));

    refresh();
    wrefresh(chat_win);
    wrefresh(input_win);
}

char *check_msg_input() {
    wclear(input_win);
    box(input_win, 0, 0);
    wattron(input_win, COLOR_PAIR(4));
    mvwprintw(input_win, 0, 2, "[ Type Message ]");
    wattroff(input_win, COLOR_PAIR(4));
    wrefresh(input_win);

    char *msg = malloc(305);
    if (!msg) {
        return NULL;
    }

    echo();
    wmove(input_win, 1, 1);

    int max_x = 0;
    int max_y = 0;
    getmaxyx(input_win, max_y, max_x);

    wgetnstr(input_win, msg, max_x - 3);

    noecho();
    return msg;
}

void print_msg(char *usrname, char *msg) {
    int max_x = 0;
    int max_y = 0;
    getmaxyx(chat_win, max_y, max_x);

    int label_len = (int)strlen(usrname) + 2;
    int max_text_width = max_x - label_len - 4;

    if (max_text_width <= 10) {
        max_text_width = 40;
    }

    if (msg_y >= max_y - 1) {
        wclear(chat_win);
        box(chat_win, 0, 0);
        mvwprintw(chat_win, 0, 2, "[ Messages ]");
        msg_y = 1;
    }
    wattron(chat_win, COLOR_PAIR(1) | A_BOLD);
    mvwprintw(chat_win, msg_y, 2, "%s:", usrname);
    wattroff(chat_win, COLOR_PAIR(1) | A_BOLD);

    char *ptr = msg;
    int remaining_len = (int)strlen(msg);
    bool first_line = true;

    while (remaining_len > 0) {
        if (msg_y >= max_y - 1) {
            wclear(chat_win);
            box(chat_win, 0, 0);
            mvwprintw(chat_win, 0, 2, "[ Messages ]");
            msg_y = 1;
            wattron(chat_win, COLOR_PAIR(1) | A_BOLD);
            mvwprintw(chat_win, msg_y, 2, "%s:", usrname);
            wattroff(chat_win, COLOR_PAIR(1) | A_BOLD);
        }

        int chunk_size =
            (remaining_len > max_text_width) ? max_text_width : remaining_len;

        char line_buffer[305] = {'\0'};
        strncpy(line_buffer, ptr, chunk_size);

        wattron(chat_win, COLOR_PAIR(3));
        if (first_line) {
            mvwprintw(chat_win, msg_y, label_len + 3, "%s", line_buffer);
            first_line = false;
        } else {
            mvwprintw(chat_win, msg_y, label_len + 3, "%s", line_buffer);
        }
        wattroff(chat_win, COLOR_PAIR(3));

        ptr += chunk_size;
        remaining_len -= chunk_size;
        msg_y++;
    }

    wrefresh(chat_win);
    wrefresh(input_win);
}

void print_announcement(char *msg) {
    int max_x = 0;
    int max_y = 0;
    getmaxyx(chat_win, max_y, max_x);

    if (msg_y >= max_y - 1) {
        wclear(chat_win);
        box(chat_win, 0, 0);
        mvwprintw(chat_win, 0, 2, "[ Messages ]");
        msg_y = 1;
    }

    wattron(chat_win, COLOR_PAIR(2) | A_ITALIC);
    mvwprintw(chat_win, msg_y, 2, "Announcement: %s", msg);
    wattroff(chat_win, COLOR_PAIR(2) | A_ITALIC);

    wrefresh(chat_win);
    wrefresh(input_win);
    msg_y++;
}
