#include <ncurses.h>
#include <string.h>
#include <ctype.h>


#include "../include/client_macros.h"
#include "../include/lengths.h"

int search_form(char *name, char *last, char *number){
    int status = SHOW_NAME;
    memset(name, '_', ENTRY_NAME_L);
    memset(last, '_', ENTRY_LAST_L);
    memset(number, '_', ENTRY_NUMBER_L);
    name[ENTRY_NAME_L] = '\0';
    last[ENTRY_LAST_L] = '\0';
    number[ENTRY_NUMBER_L] = '\0';
    mvwprintw(stdscr, 2, 1, "Name: %s   Lastname: %s  Number: %s", name, last, number);
    memset(name, '\0', ENTRY_NAME_L);
    memset(last, '\0', ENTRY_LAST_L);
    memset(number, '\0', ENTRY_NUMBER_L);

    int name_pos = 0, last_pos = 0, number_pos = 0;
    SHOW_SEND(COLOR_PAIR(2));
    wmove(stdscr, 2, 7);
    curs_set(2);
    while (1) {
        int ch = getch();
        if (ch == 27) {
            wmove(stdscr, 2, 0);
            clrtoeol();
            curs_set(0);
            wrefresh(stdscr);
            return 1;
        }
        switch (status) {
            case SHOW_NAME:
                if (ch == 9) {
                    status = SHOW_PASS_SUR;
                    wmove(stdscr, 2, 20 + ENTRY_NAME_L + last_pos);
                    wrefresh(stdscr);
                } else if (ch == KEY_BACKSPACE && name_pos) {
                    --name_pos;
                    name[name_pos] = '\0';
                    mvwaddch(stdscr, 2, 7 + name_pos, '_');
                    wmove(stdscr, 2, 7 + name_pos);
                    wrefresh(stdscr);
                } else if (isalpha(ch) && name_pos < ENTRY_NAME_L) {
                    name[name_pos] = ch;
                    mvwaddch(stdscr, 2, 7 + name_pos, ch);
                    ++name_pos;
                    wrefresh(stdscr);
                }
                break;
            case SHOW_PASS_SUR:
                if (ch == 9) {
                    status = SHOW_NUMBER;
                    wmove(stdscr, 2, 30 + ENTRY_NAME_L + ENTRY_LAST_L + number_pos);
                    wrefresh(stdscr);
                } else if (ch == KEY_BACKSPACE && last_pos) {
                    --last_pos;
                    last[last_pos] = '\0';
                    mvwaddch(stdscr, 2, 20 + ENTRY_NAME_L + last_pos, '_');
                    wmove(stdscr, 2, 20 + ENTRY_NAME_L + last_pos);
                    wrefresh(stdscr);
                } else if (isalpha(ch) && last_pos < ENTRY_LAST_L) {
                    last[last_pos] = ch;
                    mvwaddch(stdscr, 2, 20 + ENTRY_NAME_L + last_pos, ch);
                    ++last_pos;
                    wrefresh(stdscr);
                }
                break;
            case SHOW_NUMBER:
                if (ch == 9) {
                    status = SHOW_SEND;
                    SHOW_SEND(COLOR_PAIR(2) | A_REVERSE);
                    curs_set(0);
                    wrefresh(stdscr);
                } else if (ch == KEY_BACKSPACE && number_pos) {
                    --number_pos;
                    number[number_pos] = '\0';
                    mvwaddch(stdscr, 2, 30 + ENTRY_NAME_L + ENTRY_LAST_L + number_pos, '_');
                    wmove(stdscr, 2, 30 + ENTRY_NAME_L + ENTRY_LAST_L + number_pos);
                    wrefresh(stdscr);
                } else if (isdigit(ch) && number_pos < ENTRY_NUMBER_L) {
                    number[number_pos] = ch;
                    mvwaddch(stdscr, 2, 30 + ENTRY_NAME_L + ENTRY_LAST_L + number_pos, ch);
                    ++number_pos;
                    wrefresh(stdscr);
                }
                break;
            case SHOW_SEND:
                if (ch == 9) {
                    status = SHOW_NAME;
                    SHOW_SEND(COLOR_PAIR(2));
                    wmove(stdscr, 2, 7 + name_pos);
                    curs_set(2);
                    wrefresh(stdscr);
                } else if (ch == '\n' || ch == 's') {
                    wmove(stdscr, 2, 0);
                    clrtoeol();
                    curs_set(0);
                    wrefresh(stdscr);
                    return 0;
                }
                break;
        }
    }
    return 0;
}

int loggin_form(char *name, char *pass){
    int status = SHOW_NAME;
    memset(name, '_', USERNAME_L);
    memset(pass, '_', USER_PASS_L);
    name[USERNAME_L] = '\0';
    pass[USER_PASS_L] = '\0';
    mvwprintw(stdscr, 2, 1, "Name: %s   Password: %s ", name, pass);
    memset(name, '\0', USERNAME_L);
    memset(pass, '\0', USER_PASS_L);

    int name_pos = 0, pass_pos = 0;
    SHOW_SEND(COLOR_PAIR(2));
    wmove(stdscr, 2, 7);
    curs_set(2);
    while (1) {
        int ch = getch();
        if (ch == 27) {
            wmove(stdscr, 2, 0);
            clrtoeol();
            curs_set(0);
            wrefresh(stdscr);
            return 1;
        }
        switch (status) {
            case SHOW_NAME:
                if (ch == 9) {
                    status = SHOW_PASS_SUR;
                    wmove(stdscr, 2, 20 + USERNAME_L + pass_pos);
                    wrefresh(stdscr);
                } else if (ch == KEY_BACKSPACE && name_pos) {
                    --name_pos;
                    name[name_pos] = '\0';
                    mvwaddch(stdscr, 2, 7 + name_pos, '_');
                    wmove(stdscr, 2, 7 + name_pos);
                    wrefresh(stdscr);
                } else if (isalnum(ch) && name_pos < USERNAME_L) {
                    name[name_pos] = ch;
                    mvwaddch(stdscr, 2, 7 + name_pos, ch);
                    ++name_pos;
                    wrefresh(stdscr);
                }
                break;
            case SHOW_PASS_SUR:
                if (ch == 9) {
                    status = SHOW_SEND;
                    SHOW_SEND(COLOR_PAIR(2) | A_REVERSE);
                    curs_set(0);
                    wrefresh(stdscr);
                } else if (ch == KEY_BACKSPACE && pass_pos) {
                    --pass_pos;
                    pass[pass_pos] = '\0';
                    mvwaddch(stdscr, 2, 20 + USERNAME_L + pass_pos, '_');
                    wmove(stdscr, 2, 20 + USERNAME_L + pass_pos);
                    wrefresh(stdscr);
                } else if (isalnum(ch) && pass_pos < USER_PASS_L) {
                    pass[pass_pos] = ch;
                    mvwaddch(stdscr, 2, 20 + USERNAME_L + pass_pos, '*');
                    ++pass_pos;
                    wrefresh(stdscr);
                }
                break;
            case SHOW_SEND:
                if (ch == 9) {
                    status = SHOW_NAME;
                    SHOW_SEND(COLOR_PAIR(2));
                    wmove(stdscr, 2, 7 + name_pos);
                    curs_set(2);
                    wrefresh(stdscr);
                } else if (ch == '\n' || ch == 's') {
                    wmove(stdscr, 2, 0);
                    clrtoeol();
                    curs_set(0);
                    wrefresh(stdscr);
                    return 0;
                }
                break;
        }
    }
    return 0;
}

int signin_form(char *name, char *pass, char *level){
    int status = SHOW_NAME;
    memset(name, '_', USERNAME_L);
    memset(pass, '_', USER_PASS_L);
    name[USERNAME_L] = '\0';
    pass[USER_PASS_L] = '\0';
    mvwprintw(stdscr, 2, 1, "Name: %s   Password: %s  Level: user", name, pass);
    memset(name, '\0', USERNAME_L);
    memset(pass, '\0', USER_PASS_L);
    level[0] = 'u';

    int name_pos = 0, pass_pos = 0;
    SHOW_SEND(COLOR_PAIR(2));
    wmove(stdscr, 2, 7);
    curs_set(2);
    while (1) {
        int ch = getch();
        if (ch == 27) {
            wmove(stdscr, 2, 0);
            clrtoeol();
            curs_set(0);
            wrefresh(stdscr);
            return 1;
        }
        switch (status) {
            case SHOW_NAME:
                if (ch == 9) {
                    status = SHOW_PASS_SUR;
                    wmove(stdscr, 2, 20 + USERNAME_L + pass_pos);
                    wrefresh(stdscr);
                } else if (ch == KEY_BACKSPACE && name_pos) {
                    --name_pos;
                    name[name_pos] = '\0';
                    mvwaddch(stdscr, 2, 7 + name_pos, '_');
                    wmove(stdscr, 2, 7 + name_pos);
                    wrefresh(stdscr);
                } else if (isalnum(ch) && name_pos < USERNAME_L) {
                    name[name_pos] = ch;
                    mvwaddch(stdscr, 2, 7 + name_pos, ch);
                    ++name_pos;
                    wrefresh(stdscr);
                }
                break;
            case SHOW_PASS_SUR:
                if (ch == 9) {
                    status = SHOW_LEVEL;
                    attron(COLOR_PAIR(3) | A_REVERSE);
                    mvwprintw(stdscr, 2, 29 + USERNAME_L + USER_PASS_L, level[0] == 'a' ? "admin" : "user ");
                    attroff(COLOR_PAIR(3) | A_REVERSE);
                    curs_set(0);
                    wrefresh(stdscr);
                } else if (ch == KEY_BACKSPACE && pass_pos) {
                    --pass_pos;
                    pass[pass_pos] = '\0';
                    mvwaddch(stdscr, 2, 20 + USERNAME_L + pass_pos, '_');
                    wmove(stdscr, 2, 20 + USERNAME_L + pass_pos);
                    wrefresh(stdscr);
                } else if (isalnum(ch) && pass_pos < USER_PASS_L) {
                    pass[pass_pos] = ch;
                    mvwaddch(stdscr, 2, 20 + USERNAME_L + pass_pos, '*');
                    ++pass_pos;
                    wrefresh(stdscr);
                }
                break;
            case SHOW_LEVEL:
                if (ch == 9) {
                    status = SHOW_SEND;
                    mvwprintw(stdscr, 2, 29 + USERNAME_L + USER_PASS_L, level[0] == 'a' ? "admin" : "user ");
                    SHOW_SEND(COLOR_PAIR(2) | A_REVERSE);
                    curs_set(0);
                    wrefresh(stdscr);
                } else if (ch == KEY_UP || ch == KEY_DOWN) {
                    level[0] = level[0] == 'a' ? 'u' : 'a';
                    attron(COLOR_PAIR(3) | A_REVERSE);
                    mvwprintw(stdscr, 2, 29 + USERNAME_L + USER_PASS_L, level[0] == 'a' ? "admin" : "user ");
                    attroff(COLOR_PAIR(3) | A_REVERSE);
                }
                break;
            case SHOW_SEND:
                if (ch == 9) {
                    status = SHOW_NAME;
                    SHOW_SEND(COLOR_PAIR(2));
                    wmove(stdscr, 2, 7 + name_pos);
                    curs_set(2);
                    wrefresh(stdscr);
                } else if (ch == '\n' || ch == 's') {
                    wmove(stdscr, 2, 0);
                    clrtoeol();
                    curs_set(0);
                    wrefresh(stdscr);
                    return 0;
                }
                break;
        }
    }
    return 0;
}
