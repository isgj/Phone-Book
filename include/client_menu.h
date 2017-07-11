#ifndef CLIENT_MENU_H
#define CLIENT_MENU_H

#include <ncurses.h>

WINDOW *menu_win;
WINDOW *scroll_win;

#define EXP(a) (strlen(a) ? a : "*")

#define STATUS(opt, args...) do {\
    wmove(stdscr, 1, 0);\
    clrtoeol();\
    wattron(stdscr, opt);\
    mvwprintw(stdscr, 1, 1, args);\
    wattroff(stdscr, opt);\
    wrefresh(stdscr);\
} while (0)

int init_scr();
int main_menu();
int user_menu();
int admin_menu();
int add_to_swindow(int level, char *format, ...);

void end_win();
#endif
