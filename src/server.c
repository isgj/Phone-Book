#include "../include/err_ex_util.h"
#include "../include/server.h"
#include "../include/connection_handler.h"

void signal_handler(int sig) {
    server_error = 1;
    syslog(LOG_ERR, "[%lu] Got the signal %s. Closing.\n", pthread_self(), sys_siglist[sig]);

    struct sembuf oper[1];
    int ret = 0;
    alarm(WAIT_TIME_SEC);

    SEM_OPER(0, SEM_THREAD_ID, 0, 0);
    ret = semop(sem_id, oper, 1);

    alarm(0);
    if (ret == -1 && errno != EINTR) {
        syslog(LOG_ERR, "[%lu] Error waiting for threads to close.\n", pthread_self());
        syslog(LOG_ERR, "%s\n", strerror(errno));
    }

    SEM_OPER(0, SEM_NCURSES_ID, -1, 0);
    ret = semop(sem_id, oper, 1);
    if (ret == -1) {
        syslog(LOG_ERR, "[%lu] Cannot get ncurses semaphore.\n", pthread_self());
    }
    end_win();
    exit(0);
}

void sig_alarm(int sig) {
    syslog(LOG_ERR, "[%lu] Main: Some threads haven't finished yet. Closing anyway.\n", pthread_self());
}

int main(int argc, char *argv[]) {
    int ret;
    struct stat st;

    sigset_t sig_set;
    sigfillset(&sig_set);
    sigdelset(&sig_set, SIGALRM);

    struct sigaction sa_action[1];
    sa_action->sa_handler = signal_handler;
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

    sigaddset(&sig_set, SIGALRM);

    sa_action->sa_handler = SIG_IGN;
    ret = sigaction(SIGPIPE, sa_action, NULL);
    ERROR_HELPER(ret, "Cannot ignore signal SIGPIPE.\n");

    sa_action->sa_handler = sig_alarm;
    ret = sigaction(SIGALRM, sa_action, NULL);
    ERROR_HELPER(ret, "Cannot set signal handler for SIGALRM.\n");

    openlog("phone_book", LOG_CONS, LOG_LOCAL7);

    ret = init_scr();
    ERROR_HELPER(ret, "Cannot create ncurses window.\n");

    sem_id = semget(SEM_KEY, SEM_NUMBER, IPC_CREAT | IPC_EXCL | 0666);
    if (sem_id == -1) {
        sem_id = semget(SEM_KEY, SEM_NUMBER, IPC_CREAT | 0666);
        semctl(sem_id, IPC_RMID, 0);
        sem_id = semget(SEM_KEY, SEM_NUMBER, IPC_CREAT | IPC_EXCL | 0666);
        ERROR_HELPER(sem_id, "Cannot create the semaphore.\n");
    }

    ret = semctl(sem_id, SEM_NCURSES_ID, SETVAL, 1);
    ERROR_HELPER(ret, "Couldn't set the value of ncurses semaphore.\n");

    ret = semctl(sem_id, SEM_USER_ID, SETVAL, 1);
    ERROR_HELPER(ret, "Couldn't set the value of user semaphore.\n");

    ret = semctl(sem_id, SEM_REQ_ID, SETVAL, 1);
    ERROR_HELPER(ret, "Couldn't set the value of request semaphore.\n");
    LOG(LOG_INFO, "[%lu] Main: Semaphores created and initialized.\n", pthread_self());

    LOG(LOG_INFO, "[%lu] Main: Cheking files.\n", pthread_self());
    STAT_FILE(USER_FILE, ret);
    STAT_FILE(REQ_FILE, ret);
    STAT_FILE(BOOK_FILE, ret);
    LOG(LOG_INFO, "[%lu] Main: Files are ok.\n", pthread_self());

    line_num = req_number();
    set_line_num();

    int socket_desc, client_desc;

    /* clear structure */
    struct sockaddr_in server_addr = {0};

    int sockaddr_len = sizeof(struct sockaddr_in);

    LOG(LOG_INFO, "[%lu] Main: Creating the socket.\n", pthread_self());

    ret = pthread_create(&get_char, NULL, handle_input, NULL);
    PTHREAD_ERROR_HELPER(ret, "Couldn't create thread for ncurses read char.\n");

    /* get the socket */
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    ERROR_HELPER(socket_desc, "Could not create socket");

    int server_port = SERVER_PORT;
    if (argc > 1) server_port = atoi(argv[1]);

    server_addr.sin_addr.s_addr = INADDR_ANY; // we want to accept connections from any interface
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(server_port);

    LOG(LOG_INFO, "[%lu] Main: Listening on port: %d.\n", pthread_self(), server_port);

    /* We enable SO_REUSEADDR to quickly restart our server after a crash */
    int reuseaddr_opt = 1;
    ret = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_opt, sizeof(reuseaddr_opt));
    ERROR_HELPER(ret, "Cannot set SO_REUSEADDR option");

    /* bind address to socket */
    ret = bind(socket_desc, (struct sockaddr*) &server_addr, sockaddr_len);
    ERROR_HELPER(ret, "Cannot bind address to socket");

    LOG(LOG_INFO, "[%lu] Main: Listening on the sockets.\n", pthread_self());
    /* start listening */
    ret = listen(socket_desc, MAX_CONN_QUEUE);
    ERROR_HELPER(ret, "Cannot listen on socket");

    /* we allocate client_addr dynamically and initialize it to zero */
    struct sockaddr_in* client_addr = calloc(1, sizeof(struct sockaddr_in));

    /* The loop where we accept new connections */
    while (!server_error) {
        /* accept incoming connection */
        client_desc = accept(socket_desc, (struct sockaddr*) client_addr, (socklen_t*) &sockaddr_len);
        if (client_desc == -1 && errno == EINTR) continue; 
        if (client_desc == -1) {
            syslog(LOG_ERR, "Cannot open socket for incoming connection");
            signal_handler(SIGUSR2);
        }

        pthread_t thread;
        handler_args_t *arg = malloc(sizeof(handler_args_t));
        arg->socket_desc = client_desc;
        arg->client_addr = client_addr;

        ret = pthread_create(&thread, NULL, connection_handler, (void *)arg);
        PTHREAD_ERROR_HELPER(ret, "Couldn't create thread.\n");

        LOG(LOG_INFO, "[%lu] Main: New thread created.\n", pthread_self());
        pthread_detach(thread);

        client_addr = calloc(1, sizeof(struct sockaddr_in));
    }

    return 0;
}
