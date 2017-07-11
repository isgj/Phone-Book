#ifndef CLIENT_MACROS_H
#define CLIENT_MACROS_H

enum menu {SHOW_LOGGIN, SHOW_SIGNIN, SHOW_EXIT, SHOW_NEW, SHOW_UPDATE, SHOW_CANCEL, SHOW_FIND, SHOW_LOGOUT};
enum input {SHOW_NAME, SHOW_PASS_SUR, SHOW_NUMBER, SHOW_LEVEL, SHOW_SEND};

#define SHOW_LOGGIN(opt) \
    wattron(menu_win, opt);\
    mvwprintw(menu_win, 2, 1, "LOG IN");\
    wattroff(menu_win, opt);\
    if (status == SHOW_LOGGIN){\
        wattron(menu_win, opt | A_UNDERLINE);\
        mvwaddch(menu_win, 2, 1, 'L');\
        wattroff(menu_win, opt | A_UNDERLINE);\
    }
#define SHOW_SIGNIN(opt) \
    wattron(menu_win, opt);\
    mvwprintw(menu_win, 4, 1, "SIGN IN");\
    wattroff(menu_win, opt);\
    if (status == SHOW_SIGNIN){\
        wattron(menu_win, opt | A_UNDERLINE);\
        mvwaddch(menu_win, 4, 1, 'S');\
        wattroff(menu_win, opt | A_UNDERLINE);\
    }

#define SHOW_EXIT(opt) \
    wattron(menu_win, opt);\
    mvwprintw(menu_win, 6, 1, "EXIT");\
    wattroff(menu_win, opt);\
    if (status == SHOW_EXIT){\
        wattron(menu_win, opt | A_UNDERLINE);\
        mvwaddch(menu_win, 6, 1, 'E');\
        wattroff(menu_win, opt | A_UNDERLINE);\
    }

#define SHOW_SEND(opt) \
    wattron(stdscr, opt);\
    mvwprintw(stdscr, 2, COLS - 5, "SEND");\
    wattroff(stdscr, opt);\
    if (status == SHOW_PASS_SUR){\
        wattron(stdscr, opt | A_UNDERLINE);\
        mvwaddch(stdscr, 2, COLS - 5, 'S');\
        wattroff(stdscr, opt | A_UNDERLINE);\
    }

#define SHOW_FIND(opt) \
    wattron(menu_win, opt);\
    mvwprintw(menu_win, 2, 1, "SEARCH");\
    wattroff(menu_win, opt);\
    if (status == SHOW_FIND){\
        wattron(menu_win, opt | A_UNDERLINE);\
        mvwaddch(menu_win, 2, 1, 'S');\
        wattroff(menu_win, opt | A_UNDERLINE);\
    }

#define SHOW_NEW(opt) \
    wattron(menu_win, opt);\
    mvwprintw(menu_win, 4, 1, "NEW ENTRY");\
    wattroff(menu_win, opt);\
    if (status == SHOW_NEW){\
        wattron(menu_win, opt | A_UNDERLINE);\
        mvwaddch(menu_win, 4, 1, 'N');\
        wattroff(menu_win, opt | A_UNDERLINE);\
    }

#define SHOW_UPDATE(opt) \
    wattron(menu_win, opt);\
    mvwprintw(menu_win, 6, 1, "UPDATE");\
    wattroff(menu_win, opt);\
    if (status == SHOW_UPDATE){\
        wattron(menu_win, opt | A_UNDERLINE);\
        mvwaddch(menu_win, 6, 1, 'U');\
        wattroff(menu_win, opt | A_UNDERLINE);\
    }

#define SHOW_CANCEL(opt) \
    wattron(menu_win, opt);\
    mvwprintw(menu_win, 8, 1, "CANCEL");\
    wattroff(menu_win, opt);\
    if (status == SHOW_CANCEL){\
        wattron(menu_win, opt | A_UNDERLINE);\
        mvwaddch(menu_win, 8, 1, 'C');\
        wattroff(menu_win, opt | A_UNDERLINE);\
    }

#define SHOW_LOGOUT(opt) \
    wattron(menu_win, opt);\
    mvwprintw(menu_win, 14, 1, "LOG OUT");\
    wattroff(menu_win, opt);\
    if (status == SHOW_LOGOUT){\
        wattron(menu_win, opt | A_UNDERLINE);\
        mvwaddch(menu_win, 14, 1, 'L');\
        wattroff(menu_win, opt | A_UNDERLINE);\
    }

#endif
