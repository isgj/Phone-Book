#include "../include/client_menu.h"
#include "../include/client_macros.h"
#include "../include/client_forms.h"
#include "../include/client_connection.h"
#include "../include/lengths.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

extern int sock_fd;
char *user_name;
char *password;

int init_scr(){

    initscr();

    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    mvprintw(0, (COLS - 21) / 2, "THE PHONE BOOK CLIENT");
    STATUS(COLOR_PAIR(1), "Not connected.");
    menu_win = newwin(LINES - 4, 16, 3, 0);
    box(menu_win, 0, 0);

    scroll_win = newwin(LINES - 4, COLS - 17, 4, 17);
    scrollok(scroll_win, TRUE);

    if (scroll_win == NULL || menu_win == NULL) return -1;
    refresh();

    return 0;
}

int add_to_swindow(int level, char *format, ...){
    va_list args;
    va_start(args, format);

    wattron(scroll_win, COLOR_PAIR(level));
    vwprintw(scroll_win, format, args);
    wattroff(scroll_win, COLOR_PAIR(level));

    wrefresh(scroll_win);
    return 0;
}

int main_menu(){
    int status = SHOW_LOGGIN;
    int ret;
    while (1) {
        SHOW_LOGGIN(COLOR_PAIR(2) | A_REVERSE);
        SHOW_SIGNIN(COLOR_PAIR(2));
        SHOW_EXIT(COLOR_PAIR(1));

        wrefresh(menu_win);
        char name[USERNAME_L + 1];
        char pass[USER_PASS_L + 1];
        while (1) {
            int ch = getch();
            switch (status) {
                case SHOW_LOGGIN:
                    if (ch == KEY_DOWN) {
                        status = SHOW_SIGNIN;
                        SHOW_LOGGIN(COLOR_PAIR(2));
                        SHOW_SIGNIN(COLOR_PAIR(2) | A_REVERSE);
                        wrefresh(menu_win);
                    } else if (ch == KEY_UP) {
                        status = SHOW_EXIT;
                        SHOW_LOGGIN(COLOR_PAIR(2));
                        SHOW_EXIT(COLOR_PAIR(1) | A_REVERSE);
                        wrefresh(menu_win);
                    } else if (ch == 'l' || ch == '\n') {
                        status = SHOW_NAME;
                        SHOW_LOGGIN(COLOR_PAIR(2));
                        wrefresh(menu_win);

                        ret = loggin_form(name, pass);
                        if (!ret) {
                            if (strlen(name) && strlen(pass)) {
                                add_to_swindow(3, "\nTrying to log in.\n");
                                ret = loggin(name, pass);
                                if (ret != 1) {
                                    strcpy(user_name, name);
                                    strcpy(password, pass);
                                    if (ret == 'a') admin_menu();
                                    else if (ret == 'u') user_menu();
                                }
                            } else {
                                mvwprintw(stdscr, 2, 1, "Empty fields are not allowed.");
                            }
                        }
                        status = SHOW_LOGGIN;
                        SHOW_LOGGIN(COLOR_PAIR(2) | A_REVERSE);
                        SHOW_SIGNIN(COLOR_PAIR(2));
                        SHOW_EXIT(COLOR_PAIR(1));
                        wrefresh(menu_win);
                    }
                    break;
                case SHOW_SIGNIN:
                    if (ch == KEY_UP) {
                        status = SHOW_LOGGIN;
                        SHOW_LOGGIN(COLOR_PAIR(2) | A_REVERSE);
                        SHOW_SIGNIN(COLOR_PAIR(2));
                        wrefresh(menu_win);
                    } else if (ch == KEY_DOWN) {
                        status = SHOW_EXIT;
                        SHOW_SIGNIN(COLOR_PAIR(2));
                        SHOW_EXIT(COLOR_PAIR(1) | A_REVERSE);
                        wrefresh(menu_win);
                    } else if (ch == 's' || ch == '\n') {
                        char name[USERNAME_L + 1];
                        char pass[USER_PASS_L + 1];
                        char level;
                        ret = signin_form(name, pass, &level);
                        if (!ret) {
                            if (!strlen(name) || !strlen(pass))
                            mvwprintw(stdscr, 2, 1, "Empty fields are not allowed.");
                            else {
                                char msg[MSG_BUF_LEN];
                                sprintf(msg, "ME_NEW %s %s %c\n", name, pass, level);
                                add_to_swindow(3, "\nTrying to sign in.\n");
                                if (send_get(msg)) add_to_swindow(1, "Connection error.\n");
                                close(sock_fd);
                                STATUS(COLOR_PAIR(1), "Not connected.");
                            }
                        }
                    }
                    break;
                case SHOW_EXIT:
                    if (ch == KEY_DOWN) {
                        status = SHOW_LOGGIN;
                        SHOW_LOGGIN(COLOR_PAIR(2) | A_REVERSE);
                        SHOW_EXIT(COLOR_PAIR(1));
                        wrefresh(menu_win);
                    } else if (ch == KEY_UP) {
                        status = SHOW_SIGNIN;
                        SHOW_SIGNIN(COLOR_PAIR(2) | A_REVERSE);
                        SHOW_EXIT(COLOR_PAIR(1));
                        wrefresh(menu_win);
                    } else if (ch == 'e' || ch == '\n') {
                        add_to_swindow(2, "%s", "Time to exit.\n");
                        return 0;
                    }
                    break;
            }
        }
    }
}

int user_menu(){
    mvwprintw(menu_win, 4, 1, "       ");
    mvwprintw(menu_win, 6, 1, "       ");
    int status = SHOW_FIND;
    SHOW_FIND(COLOR_PAIR(2) | A_REVERSE);
    SHOW_LOGOUT(COLOR_PAIR(1));
    wrefresh(menu_win);
    char msg_buf[MSG_BUF_LEN];
    while (1) {
        int ch = getch();
        switch (status) {
            case SHOW_FIND:
                if (ch == KEY_UP || ch == KEY_DOWN) {
                    status = SHOW_LOGOUT;
                    SHOW_FIND(COLOR_PAIR(2));
                    SHOW_LOGOUT(COLOR_PAIR(1) | A_REVERSE);
                    wrefresh(menu_win);
                } else if (ch == '\n' || ch == 's') {
                    char name[ENTRY_NAME_L + 1];
                    char last[ENTRY_LAST_L + 1];
                    char number[ENTRY_NUMBER_L + 1];
                    int ret = search_form(name, last, number);
                    if(!ret) {
                        sprintf(msg_buf, "FIND_EXP %s %s %s\n", EXP(name), EXP(last), EXP(number));
                        add_to_swindow(3, "\nSearching....\n");
                        if (send_get(msg_buf)) add_to_swindow(1, "Connection error.\n");
                    }
                }
                break;
            case SHOW_LOGOUT:
                if (ch == KEY_UP || ch == KEY_DOWN) {
                    status = SHOW_FIND;
                    SHOW_FIND(COLOR_PAIR(2) | A_REVERSE);
                    SHOW_LOGOUT(COLOR_PAIR(1));
                    wrefresh(menu_win);
                } else if (ch == '\n' || ch == 'l') {
                    add_to_swindow(3, "\nLogging out.\n");
                    if (send_get("ME_OUT\n")) add_to_swindow(1, "Connection error.\n");
                    user_name[0] = '\0';
                    mvwprintw(menu_win, 14, 1, "       ");
                    close(sock_fd);
                    STATUS(COLOR_PAIR(1), "Not connected.");
                    return 0;
                }
                break;
        }
    }
    return 0;
}

int admin_menu(){
    int status = SHOW_FIND;
    SHOW_FIND(COLOR_PAIR(2) | A_REVERSE);
    SHOW_NEW(COLOR_PAIR(2));
    SHOW_UPDATE(COLOR_PAIR(2));
    SHOW_CANCEL(COLOR_PAIR(2));
    SHOW_LOGOUT(COLOR_PAIR(1));
    wrefresh(menu_win);
    char msg_buf[MSG_BUF_LEN];
    while (1) {
        int ch = getch();
        switch (status) {
            case SHOW_FIND:
                if (ch == KEY_UP) {
                    status = SHOW_LOGOUT;
                    SHOW_LOGOUT(COLOR_PAIR(1) | A_REVERSE);
                    SHOW_FIND(COLOR_PAIR(2));
                    wrefresh(menu_win);
                } else if (ch == KEY_DOWN) {
                    status = SHOW_NEW;
                    SHOW_FIND(COLOR_PAIR(2));
                    SHOW_NEW(COLOR_PAIR(2) | A_REVERSE);
                    wrefresh(menu_win);
                } else if (ch == '\n' || ch == 's') {
                    char name[ENTRY_NAME_L + 1];
                    char last[ENTRY_LAST_L + 1];
                    char number[ENTRY_NUMBER_L + 1];
                    int ret = search_form(name, last, number);
                    if(!ret) {
                        sprintf(msg_buf, "FIND_EXP %s %s %s\n", EXP(name), EXP(last), EXP(number));
                        add_to_swindow(3, "\nSearching....\n");
                        if (send_get(msg_buf)) add_to_swindow(1, "Connection error.\n");
                    }
                }
                break;
            case SHOW_NEW:
                if (ch == KEY_UP) {
                    status = SHOW_FIND;
                    SHOW_FIND(COLOR_PAIR(2) | A_REVERSE);
                    SHOW_NEW(COLOR_PAIR(2));
                    wrefresh(menu_win);
                } else if (ch == KEY_DOWN) {
                    status = SHOW_UPDATE;
                    SHOW_NEW(COLOR_PAIR(2));
                    SHOW_UPDATE(COLOR_PAIR(2) | A_REVERSE);
                    wrefresh(menu_win);
                } else if (ch == '\n' || ch == 'n') {
                    char name[ENTRY_NAME_L + 1];
                    char last[ENTRY_LAST_L + 1];
                    char number[ENTRY_NUMBER_L + 1];
                    int ret = search_form(name, last, number);
                    if(!ret) {
                        if (strlen(name) && strlen(last) && strlen(number)) {
                            sprintf(msg_buf, "ADD_THIS %s %s %s\n", name, last, number);
                            add_to_swindow(3, "\nAdding new entry.\n");
                            if (send_get(msg_buf)) add_to_swindow(1, "Connection error.\n");
                        } else
                            mvwprintw(stdscr, 2, 1, "Empty fields are not allowed.");
                    }
                }
                break;
            case SHOW_UPDATE:
                if (ch == KEY_UP) {
                    status = SHOW_NEW;
                    SHOW_NEW(COLOR_PAIR(2) | A_REVERSE);
                    SHOW_UPDATE(COLOR_PAIR(2));
                    wrefresh(menu_win);
                } else if (ch == KEY_DOWN) {
                    status = SHOW_CANCEL;
                    SHOW_UPDATE(COLOR_PAIR(2));
                    SHOW_CANCEL(COLOR_PAIR(2) | A_REVERSE);
                    wrefresh(menu_win);
                } else if (ch == '\n' || ch == 'u') {
                    char name[ENTRY_NAME_L + 1];
                    char last[ENTRY_LAST_L + 1];
                    char number[ENTRY_NUMBER_L + 1];
                    int ret = search_form(name, last, number);
                    if(!ret) {
                        if (strlen(name) && strlen(last) && strlen(number)) {
                            sprintf(msg_buf, "UPDATE %s %s %s\n", name, last, number);
                            add_to_swindow(3, "\nUpdating...\n");
                            if (send_get(msg_buf)) add_to_swindow(1, "Connection error.\n");
                        } else
                            mvwprintw(stdscr, 2, 1, "Empty fields are not allowed.");
                    }
                }
                break;
            case SHOW_CANCEL:
                if (ch == KEY_UP) {
                    status = SHOW_UPDATE;
                    SHOW_UPDATE(COLOR_PAIR(2) | A_REVERSE);
                    SHOW_CANCEL(COLOR_PAIR(2));
                    wrefresh(menu_win);
                } else if (ch == KEY_DOWN) {
                    status = SHOW_LOGOUT;
                    SHOW_CANCEL(COLOR_PAIR(2));
                    SHOW_LOGOUT(COLOR_PAIR(1) | A_REVERSE);
                    wrefresh(menu_win);
                } else if (ch == '\n' || ch == 'c') {
                    char name[ENTRY_NAME_L + 1];
                    char last[ENTRY_LAST_L + 1];
                    char number[ENTRY_NUMBER_L + 1];
                    int ret = search_form(name, last, number);
                    if(!ret) {
                        if (strlen(name) && strlen(last) && strlen(number)) {
                            sprintf(msg_buf, "CANCEL %s %s %s\n", name, last, EXP(number));
                            add_to_swindow(3, "\nCanceling...\n");
                            if (send_get(msg_buf)) add_to_swindow(1, "Connection error.\n");
                        } else
                            mvwprintw(stdscr, 2, 1, "Empty fields are not allowed.");
                    }
                }
                break;
            case SHOW_LOGOUT:
                if (ch == KEY_UP) {
                    status = SHOW_CANCEL;
                    SHOW_CANCEL(COLOR_PAIR(2) | A_REVERSE);
                    SHOW_LOGOUT(COLOR_PAIR(1));
                    wrefresh(menu_win);
                } else if(ch == KEY_DOWN){
                    status = SHOW_FIND;
                    SHOW_LOGOUT(COLOR_PAIR(1));
                    SHOW_FIND(COLOR_PAIR(2) | A_REVERSE);
                    wrefresh(menu_win);
                } else if (ch == '\n' || ch == 'l') {
                    add_to_swindow(3, "\nLogging out.\n");
                    if (send_get("ME_OUT\n")) add_to_swindow(1, "Connection error.\n");
                    user_name[0] = '\0';
                    mvwprintw(menu_win, 4, 1, "         ");
                    mvwprintw(menu_win, 6, 1, "      ");
                    mvwprintw(menu_win, 8, 1, "      ");
                    mvwprintw(menu_win, 14, 1, "       ");

                    close(sock_fd);
                    STATUS(COLOR_PAIR(1), "Not connected.");
                    return 0;
                }
                break;
        }
    }
    return 0;
}
void end_win(){
    delwin(scroll_win);
    delwin(menu_win);
    endwin();
}
