#ifndef MY_GUI_H
#define MY_GUI_H

#include <ncurses.h>
#include <panel.h>

extern int server_error;
extern int my_t_total;
extern int my_u_logged;
extern int my_a_logged;

enum colors {ERROR, INFO, TIME};
enum logged_user {G_UNKNOWN, G_ADMIN, G_USER};
enum {EXIT_BUTTON, SHOW_BUTTON, DIALOG_Y, DIALOG_N};

WINDOW *curses_win;
PANEL *curses_panel;
WINDOW *panel_win;
PANEL *panel;

#define EXIT_THREAD_G(msg) do {\
    syslog(LOG_ERR, "[%lu] %s\n", pthread_self(), msg);\
    pthread_exit(NULL);\
} while (0)

#define EXIT_THREAD_E(msg) do {\
    syslog(LOG_ERR, "[%lu] %s\n", pthread_self(), msg);\
    server_error = 1;\
    kill(getpid(), SIGUSR1);\
    pthread_exit(NULL);\
} while (0)

#define SEM_DO_G() \
    while (semop(sem_id, oper, 1) == -1) {\
        if (server_error) return -1;\
        if (errno == EINTR) continue;\
        syslog(LOG_ERR, "[%lu] Cannot operate on the ncurses semaphore.\n", pthread_self());\
        syslog(LOG_ERR, "%s", strerror(errno));\
        server_error = 1;\
        kill(getpid(), SIGUSR1);\
        return -1;\
    }

#define SEM_DO_G_T() \
    while (semop(sem_id, oper, 1) == -1) {\
        if (errno == EINTR) continue;\
        server_error = 1;\
        kill(getpid(), SIGUSR1);\
        EXIT_THREAD_G("Cannot operate on the ncurses semaphore.");\
    }

#define SHOW_EXIT(opt) \
    wattron(stdscr, opt);\
    mvwprintw(stdscr, 4, 60, "EXIT");\
    wattroff(stdscr, opt);\
    if (status == EXIT_BUTTON){\
        wattron(stdscr, opt | A_UNDERLINE);\
        mvwaddch(stdscr, 4, 60, 'E');\
        wattroff(stdscr, opt | A_UNDERLINE);\
    }

#define SHOW_REQ(opt) \
    wattron(stdscr, opt);\
    mvwprintw(stdscr, 4, 10, "Show request");\
    wattroff(stdscr, opt);\
    if (status == SHOW_BUTTON){\
        wattron(stdscr, opt | A_UNDERLINE);\
        mvwaddch(stdscr, 4, 10, 'S');\
        wattroff(stdscr, opt | A_UNDERLINE);\
    }


#define HIDE_REQ() mvwprintw(stdscr, 4, 10, "            ");

#define SHOW_YES(opt) \
    wattron(panel_win, opt);\
    mvwprintw(panel_win, 5, yes_pos, " YES ");\
    wattroff(panel_win, opt);\
    if (status == DIALOG_Y){\
        wattron(panel_win, opt | A_UNDERLINE);\
        mvwaddch(panel_win, 5, yes_pos + 1, 'Y');\
        wattroff(panel_win, opt | A_UNDERLINE);\
    }

#define SHOW_NO(opt) \
    wattron(panel_win, opt);\
    mvwprintw(panel_win, 5, no_pos, " NO ");\
    wattroff(panel_win, opt);\
    if (status == DIALOG_N){\
        wattron(panel_win, opt | A_UNDERLINE);\
        mvwaddch(panel_win, 5, no_pos + 1, 'N');\
        wattroff(panel_win, opt | A_UNDERLINE);\
    }

int init_scr();
int set_line_num();
void * handle_input(void *args);
void handle_user_req();
int add_to_swindow(int level, char *format, ...);
int new_user(enum logged_user user);
int out_user(enum logged_user user);
void show_button_handler(char *msg, int status, int *yes, int *no);
void end_win();
#endif
