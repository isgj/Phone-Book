#include "../include/gui.h"
#include "../include/worker.h"
#include "../include/err_ex_util.h"

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

    int row, col;
    getmaxyx(stdscr, row, col);
    mvprintw(0, (col - 14) / 2, "THE PHONE BOOK");
    mvprintw(1, 1, "Server status: %4d tot, %4d admins, %4d users, %4d not logged.", 0, 0, 0, 0);
    curses_win = newwin(row - 7, col, 6, 1);
    new_panel(stdscr);
    curses_panel = new_panel(curses_win);
    scrollok(curses_win, TRUE);

    panel_win = newwin(1, 1, 1, 1);
    panel = new_panel(panel_win);
    if (curses_win == NULL || panel_win == NULL || curses_panel == NULL || panel == NULL) return -1;

    hide_panel(panel);
    int status = EXIT_BUTTON;
    SHOW_EXIT((COLOR_PAIR(1) | A_REVERSE));


    update_panels();
    doupdate();

    return 0;
}

int set_line_num(){
    struct sembuf oper[1];
    SEM_OPER(0, SEM_NCURSES_ID, -1, 0);
    SEM_DO_G();
    int status = -1;
    move(3, 0);
    clrtoeol();
    switch (line_num) {
        case 0:
            mvwprintw(stdscr, 3, 1, "There are no requests for new users.\n");
            HIDE_REQ();
            break;
        case 1:
            mvwprintw(stdscr, 3, 1, "There is 1 new request.\n");
            SHOW_REQ(COLOR_PAIR(2));
            break;
        default:
            mvwprintw(stdscr, 3, 1, "There are %d new requests.\n", line_num);
            SHOW_REQ(COLOR_PAIR(2));
            break;
    }
    wrefresh(stdscr);
    SEM_OPER(0, SEM_NCURSES_ID, 1, 0);
    SEM_DO_G();
    return 0;
}

void * handle_input(void *args){
    enum {EXIT_BUTTON, SHOW_BUTTON, DIALOG_Y, DIALOG_N};
    int ch, status;
    status = EXIT_BUTTON;
    char msg_buf[MSG_BUF_LEN];
    int yes_pos, no_pos;

    struct sembuf oper[1];
    while (1) {
        ch = getch();
        switch (status) {
            case EXIT_BUTTON:
                if (ch == 'e' || ch == '\n') {
                    syslog(LOG_INFO, "[%lu] Closing the server.\n", pthread_self());
                    server_error = 1;
                    kill(getpid(), SIGUSR1);
                    // pthread_exit(EXIT_SUCCESS);
                }
                if (ch == KEY_LEFT && line_num > 0) {
                    status = SHOW_BUTTON;
                    SEM_OPER(0, SEM_NCURSES_ID, -1, 0);
                    SEM_DO_G_T();
                    SHOW_EXIT(COLOR_PAIR(1));
                    SHOW_REQ((COLOR_PAIR(2) | A_REVERSE));
                    wrefresh(stdscr);
                    SEM_OPER(0, SEM_NCURSES_ID, 1, 0);
                    SEM_DO_G_T();
                }
                break;
            case SHOW_BUTTON:
                if (ch == 's' || ch == '\n') {
                    status = DIALOG_Y;
                    show_button_handler(msg_buf, status, &yes_pos, &no_pos);
                }
                if (ch == KEY_RIGHT) {
                    status = EXIT_BUTTON;
                    SEM_OPER(0, SEM_NCURSES_ID, -1, 0);
                    SEM_DO_G_T();
                    SHOW_EXIT((COLOR_PAIR(1) | A_REVERSE));
                    SHOW_REQ(COLOR_PAIR(2));
                    wrefresh(stdscr);
                    SEM_OPER(0, SEM_NCURSES_ID, 1, 0);
                    SEM_DO_G_T();
                }
                break;
            case DIALOG_Y:
            case DIALOG_N:
                if ((ch == 'y' && status == DIALOG_Y) || (ch == 'n' && status == DIALOG_N) || ch == '\n') {
                    int promote = status == DIALOG_Y ? 1 : 0;
                    SEM_OPER(0, SEM_REQ_ID, -1, 0);
                    SEM_DO_G_T();
                    if (promote) {
                        SEM_OPER(0, SEM_USER_ID, -1, 0);
                        SEM_DO_G_T();
                    }
                    if (promote_first_req(msg_buf, promote) == -1) EXIT_THREAD_G("Found error on promote_first_req.");
                    line_num--;
                    SEM_OPER(0, SEM_REQ_ID, 1, 0);
                    SEM_DO_G_T();
                    if (promote) {
                        SEM_OPER(0, SEM_USER_ID, 1, 0);
                        SEM_DO_G_T();
                    }
                    if (set_line_num() == -1) EXIT_THREAD_G("Found error on set_line_num.");
                    status = EXIT_BUTTON;
                    SEM_OPER(0, SEM_NCURSES_ID, -1, 0);
                    SEM_DO_G_T();
                    SHOW_EXIT(COLOR_PAIR(1) | A_REVERSE);
                    hide_panel(panel);
                    //top_panel(curses_panel);
                    update_panels();
                    doupdate();
                    SEM_OPER(0, SEM_NCURSES_ID, 1, 0);
                    SEM_DO_G_T();
                }
                if (ch == KEY_RIGHT && status == DIALOG_Y){
                    status = DIALOG_N;
                    SEM_OPER(0, SEM_NCURSES_ID, -1, 0);
                    SEM_DO_G_T();
                    SHOW_NO(COLOR_PAIR(1) | A_REVERSE);
                    SHOW_YES(COLOR_PAIR(2));
                    update_panels();
                    doupdate();
                    SEM_OPER(0, SEM_NCURSES_ID, 1, 0);
                    SEM_DO_G_T();
                }
                if (ch == KEY_LEFT && status == DIALOG_N){
                    status = DIALOG_Y;
                    SEM_OPER(0, SEM_NCURSES_ID, -1, 0);
                    SEM_DO_G_T();
                    SHOW_NO(COLOR_PAIR(1));
                    SHOW_YES(COLOR_PAIR(2) | A_REVERSE);
                    update_panels();
                    doupdate();
                    SEM_OPER(0, SEM_NCURSES_ID, 1, 0);
                    SEM_DO_G_T();
                }
                if (ch == 27) {
                    status = SHOW_BUTTON;
                    SEM_OPER(0, SEM_NCURSES_ID, -1, 0);
                    SEM_DO_G_T();

                    hide_panel(panel);
                    SHOW_REQ(COLOR_PAIR(2) | A_REVERSE);
                    hide_panel(panel);
                    update_panels();
                    doupdate();

                    SEM_OPER(0, SEM_NCURSES_ID, 1, 0);
                    SEM_DO_G_T();
                }
                break;
        }
    }

    return NULL;
}

void handle_user_req(){

}

int add_to_swindow(int level, char *format, ...){

    va_list args;
    va_start(args, format);

    time_t timer;
    timer = time(NULL);
    char *currentTime = ctime(&timer);
    currentTime[strlen(currentTime)-1] = '\0';

    struct sembuf oper[1];
    SEM_OPER(0, SEM_NCURSES_ID, -1, 0);
    SEM_DO_G();
    wattron(curses_win, COLOR_PAIR(3));
    wprintw(curses_win, "(%s) ", currentTime);
    wattroff(curses_win, COLOR_PAIR(3));

    level = level == LOG_ERR ? ERROR +1 : INFO + 1;
    wattron(curses_win, COLOR_PAIR(level));
    vwprintw(curses_win, format, args);
    wattroff(curses_win, COLOR_PAIR(level));

    update_panels();
    doupdate();
    SEM_OPER(0, SEM_NCURSES_ID, 1, 0);
    SEM_DO_G()
    return 0;
}

int new_user(enum logged_user user){
    struct sembuf oper[1];
    SEM_OPER(0, SEM_NCURSES_ID, -1, 0);
    SEM_DO_G();
    switch (user) {
        case G_UNKNOWN:
            ++my_t_total;
            wattron(stdscr, COLOR_PAIR(3) | A_BOLD);
            mvwprintw(stdscr, 1, 16, "%4d", my_t_total);
            wattroff(stdscr, COLOR_PAIR(3) | A_BOLD);
            break;
        case G_ADMIN:
            ++my_a_logged;
            wattron(stdscr, COLOR_PAIR(2) | A_BOLD);
            mvwprintw(stdscr, 1, 26, "%4d", my_a_logged);
            wattroff(stdscr, COLOR_PAIR(2) | A_BOLD);

            break;
        case G_USER:
            ++my_u_logged;
            wattron(stdscr, COLOR_PAIR(2) | A_BOLD);
            mvwprintw(stdscr, 1, 39, "%4d", my_u_logged);
            wattroff(stdscr, COLOR_PAIR(2) | A_BOLD);

            break;
    }
    wattron(stdscr, COLOR_PAIR(1) | A_BOLD);
    mvwprintw(stdscr, 1, 51, "%4d", my_t_total - my_a_logged - my_u_logged);
    wattroff(stdscr, COLOR_PAIR(1) | A_BOLD);
    wrefresh(stdscr);
    SEM_OPER(0, SEM_NCURSES_ID, 1, 0);
    SEM_DO_G();
    return 0;
}

int out_user(enum logged_user user){
    struct sembuf oper[1];
    SEM_OPER(0, SEM_NCURSES_ID, -1, 0);
    SEM_DO_G();
    --my_t_total;
    wattron(stdscr, COLOR_PAIR(3) | A_BOLD);
    mvwprintw(stdscr, 1, 16, "%4d", my_t_total);
    wattroff(stdscr, COLOR_PAIR(3) | A_BOLD);
    switch (user) {
        case G_UNKNOWN:
            wattron(stdscr, COLOR_PAIR(1) | A_BOLD);
            mvwprintw(stdscr, 1, 51, "%4d", my_t_total - my_a_logged - my_u_logged);
            wattroff(stdscr, COLOR_PAIR(1) | A_BOLD);
            break;
        case G_ADMIN:
            --my_a_logged;
            wattron(stdscr, COLOR_PAIR(2) | A_BOLD);
            mvwprintw(stdscr, 1, 26, "%4d", my_a_logged);
            wattroff(stdscr, COLOR_PAIR(3) | A_BOLD);
            break;
        case G_USER:
            --my_u_logged;
            wattron(stdscr, COLOR_PAIR(2) | A_BOLD);
            mvwprintw(stdscr, 1, 39, "%4d", my_u_logged);
            wattron(stdscr, COLOR_PAIR(2) | A_BOLD);
            break;
    }
    wrefresh(stdscr);
    SEM_OPER(0, SEM_NCURSES_ID, 1, 0);
    SEM_DO_G();
    return 0;
}

void show_button_handler(char * msg, int status, int *y_pos, int *n_pos){
    struct sembuf oper[1];
    char msg_str[MSG_BUF_LEN];

    SEM_OPER(0, SEM_REQ_ID, -1, 0);
    SEM_DO_G_T();
    if (get_a_request(msg) == -1) EXIT_THREAD_G("Found error in get_a_request");
    SEM_OPER(0, SEM_REQ_ID, 1, 0);
    SEM_DO_G_T();

    char *level = msg[strlen(msg) - 2] == 'a' ? "an administrator" : "a simplse user";
    char *name = strtok(msg, " ");
    sprintf(msg_str, "'%s' wants to become %s.", name, level);
    msg[strlen(msg)] = ' ';

    int len = strlen(msg_str) + 2;
    int yes_pos = len / 2 - 7;
    int no_pos = len / 2 + 2;
    *y_pos = yes_pos;
    *n_pos = no_pos;

    SEM_OPER(0, SEM_NCURSES_ID, -1, 0);
    SEM_DO_G_T();
    WINDOW *old_win = panel_window(panel);
	panel_win = newwin(7, len, (LINES - 7) / 2, (COLS - len + 2) / 2);
    if (panel_win == NULL) EXIT_THREAD_E("Cannot create new dialog panel.\n");
	replace_panel(panel, panel_win);
	box(panel_win, 0, 0);
	delwin(old_win);

    mvwprintw(panel_win, 1, 1, msg_str);
    mvwprintw(panel_win, 3, 10, "Do you accept ?");
    SHOW_YES((COLOR_PAIR(2) | A_REVERSE));
    SHOW_NO(COLOR_PAIR(1));
    SHOW_REQ(COLOR_PAIR(2));

    top_panel(panel);
    update_panels();
    doupdate();
    SEM_OPER(0, SEM_NCURSES_ID, 1, 0);
    SEM_DO_G_T();
}

void end_win(){
    del_panel(panel);
    del_panel(curses_panel);
    delwin(curses_win);
    delwin(panel_win);
    delwin(curses_win);
    endwin();
}
