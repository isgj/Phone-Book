#ifndef ERR_EX_UTIL_H
#define ERR_EX_UTIL_H

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <pthread.h>
#include "gui.h"
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>

#define SEM_KEY 127
#define SEM_NCURSES_ID 0
#define SEM_THREAD_ID 1
#define SEM_WRITE_ID 2
#define SEM_READ_ID 3
#define SEM_USER_ID 4
#define SEM_REQ_ID 5
/* The number of semaphores */
#define SEM_NUMBER 6
int sem_id;

int line_num;
pthread_t get_char;
#define SEM_OPER(index, sem_id, opt, flag)  do {\
    oper[index].sem_num = sem_id;\
    oper[index].sem_op = opt;\
    oper[index].sem_flg = flag;\
} while (0)


#define GENERIC_ERROR_HELPER(cond, errCode, msg) do {\
    if (cond) {\
        if(curses_win != NULL) end_win();\
        fprintf(stderr, "%s", msg);\
        fprintf(stderr, "%s\n", strerror(errCode));\
        syslog(LOG_ERR, msg);\
        syslog(LOG_ERR, "%s", strerror(errCode));\
        closelog();\
        exit(EXIT_FAILURE);\
    }\
} while(0)

#define ERROR_HELPER(ret, msg)          GENERIC_ERROR_HELPER((ret < 0), errno, msg)
#define PTHREAD_ERROR_HELPER(ret, msg)  GENERIC_ERROR_HELPER((ret != 0), ret, msg)

#define LOG(prior, message, args...) do {\
    syslog(prior, message, args);\
    add_to_swindow(prior, message, args);\
} while (0)

#define WORKER_ERROR(message, args...) do {\
    sprintf(msg_buf, INT_ERROR);\
    LOG(LOG_ERR, message, args);\
    LOG(LOG_ERR, "[%lu] %s\n", pthread_self(), strerror(errno));\
    return -1;\
} while (0)

#define CLOSE_THREAD(a) do {\
    if (a) {\
        server_error = 1;\
        kill(getpid(), SIGUSR1);\
        LOG(LOG_ERR, "[%lu] Thread found error. Exiting.\n", pthread_self());\
    } else {\
        LOG(LOG_INFO, "[%lu] Thread exiting gracefully.\n", pthread_self());\
    }\
    out_user(user.logged == OUT ? G_UNKNOWN : (user.level == ADMIN ? G_ADMIN : G_USER));\
    SEM_OPER(0, SEM_THREAD_ID, -1, 0);\
    while (semop(sem_id, oper, 1) == -1) {\
        if (errno == EINTR) continue;\
        LOG(LOG_ERR, "[%lu] Cannot operate on the %s semaphore.\n", pthread_self(), "thread");\
        LOG(LOG_ERR, "%s", strerror(errno));\
        server_error = 1;\
        kill(getpid(), SIGUSR1);\
    }\
    pthread_exit(NULL);\
} while (0)

#endif
