#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

#include "../include/client.h"
#include "../include/lengths.h"
#include "../include/client_menu.h"
#include "../include/client_connection.h"


char *server_ip;
int server_port = SERVER_PORT;
int sock_fd;
char *user_name;
char *password;

int main(int argc, char *argv[]) {
    int ret;

    sigset_t sig_set;
    sigfillset(&sig_set);

    struct sigaction sa_action[1];
    sa_action->sa_handler = sig_handler;
    sa_action->sa_mask = sig_set;

    ret = sigaction(SIGUSR1, sa_action, NULL);
    ERROR_HELPER(ret, "Cannot set signal handler for SIGUSR1.\n");

    ret = sigaction(SIGBUS, sa_action, NULL);
    ERROR_HELPER(ret, "Cannot set signal handler for SIGBUS.\n");

    ret = sigaction(SIGILL, sa_action, NULL);
    ERROR_HELPER(ret, "Cannot set signal handler for SIGILL.\n");

    ret = sigaction(SIGINT, sa_action, NULL);
    ERROR_HELPER(ret, "Cannot set signal handler for SIGINT.\n");

    ret = sigaction(SIGQUIT, sa_action, NULL);
    ERROR_HELPER(ret, "Cannot set signal handler for SIGQUIT.\n");

    ret = sigaction(SIGTERM, sa_action, NULL);
    ERROR_HELPER(ret, "Cannot set signal handler for SIGTERM.\n");

    sa_action->sa_handler = sig_alarm;
    ret = sigaction(SIGALRM, sa_action, NULL);
    ERROR_HELPER(ret, "Cannot set signal handler for SIGTERM.\n");


    sa_action->sa_handler = sig_pipe;
    ret = sigaction(SIGPIPE, sa_action, NULL);
    ERROR_HELPER(ret, "Cannot set signal SIGPIPE.\n");

    user_name = malloc(USERNAME_L + 1);
    password = malloc(USER_PASS_L + 1);
    ret = init_scr();
    ERROR_HELPER(ret, "Cannot create ncurses window.\n");

    if (argc > 1) server_ip = argv[1]; else server_ip = SERVER_IP;
    if (argc > 2) server_port = atoi(argv[2]);

    user_name[0] = '\0';
    password[0] = '\0';
    add_to_swindow(2, "Server ip : %s, server port: %d.\n", server_ip, server_port);

    main_menu();
    end_win();

    return 0;
}
