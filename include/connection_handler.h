#ifndef CONNECTION_HANDLER_H
#define CONNECTION_HANDLER_H

#include "worker.h"

typedef struct handler_args_s {
    int socket_desc;
    struct sockaddr_in *client_addr;
} handler_args_t;

#define MAX_CONN_QUEUE 30
#define SERVER_PORT 9090
#define SEC_TO_WIAT 60
#define NSEC_TO_WAIT 0

#define CHECK_LENGTH(a, b) ((a) != NULL && (strlen(a) <= (b)))
#define CHECK_LEVEL(a) ((a) != NULL && (strlen(a) == 1 && (*a == 'a' || *a == 'u')))
#define CHECK_ALPHA(a, b) (CHECK_LENGTH(a, b) && check_alpha(a))
#define CHECK_DIGIT(a, b) (CHECK_LENGTH(a, b) && check_digit(a))
#define CHECK_ALNUM(a, b) (CHECK_LENGTH(a, b) && check_alnum(a))
#define CHECK_EXP_ALPHA(a, b) (((a) != NULL && (strlen(a) == 1 && *(a) == '*')) || (CHECK_LENGTH(a, b) && check_alpha(a)))
#define CHECK_EXP_DIGIT(a, b) (((a) != NULL && (strlen(a) == 1 && *(a) == '*')) || (CHECK_LENGTH(a, b) && check_digit(a)))

#define BAD_REQUEST "Your request is not well-formated.\n\n"
#define BAD_CMD "I didn't get your request. You can check our API or use our client.\n\n"
#define NEED_LOGGIN "You need to log in first.\n\n"
#define NEED_LOGGOUT "You can't make this request since you are already logged in.\n\n"
#define NO_PERMISION "You do not have the permision to complete this task.\n\n"

#define SEM_DO(num, name) \
    while (semop(sem_id, oper, num) == -1) {\
        if (server_error) return -1;\
        if (errno == EINTR) continue;\
        LOG(LOG_ERR, "[%lu] Cannot operate on the %s semaphore.\n", pthread_self(), name);\
        LOG(LOG_ERR, "%s", strerror(errno));\
        sprintf(msg_buf, INT_ERROR);\
        server_error = 1;\
        kill(getpid(), SIGUSR1);\
        return -1;\
    }

void * connection_handler(void *args);
int proccess_command(struct message_s *message, struct user_s *user, char *msg_buf, const int sock);
int get_cmd(const char *cmd);
void thread_alarm(int sig);
int check_alpha(char*string);
int check_digit(char*string);
int check_alnum(char*string);
#endif
