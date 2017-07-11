#include <ctype.h>
#include "../include/err_ex_util.h"
#include "../include/connection_handler.h"
#include "../include/lengths.h"

/* Macro to free the resources */
#define FREE_RES() do {\
    if (shutdown(arg->socket_desc, 0) < 0)\
        LOG(LOG_ERR, "[%lu] Cannot shutdown the socket.\n", pthread_self());\
    if (close(arg->socket_desc) < 0)\
        LOG(LOG_ERR, "[%lu] Cannot close the socket.\n", pthread_self());\
    free(arg->client_addr);\
    free(arg);\
    free(msg_buf);\
} while (0)

/* Macro to send from msg_buf to socket */
#define SEND(msg_len, bytes_sent, ret) do {\
    msg_len = strlen(msg_buf);\
    bytes_sent = 0;\
    while (bytes_sent < msg_len) {\
        ret = send(arg->socket_desc, msg_buf + bytes_sent, msg_len - bytes_sent, 0);\
        if (ret == -1 && errno == EINTR) continue;\
        if (ret < 0) {\
            /* Found error */\
            FREE_RES();\
            if (errno == EAGAIN || errno == EWOULDBLOCK) {\
                LOG(LOG_INFO, "[%lu] Socket timeoout.\n", pthread_self());\
                CLOSE_THREAD(0);\
            }\
            LOG(LOG_ERR, "[%lu] Error sending to the socket\n", pthread_self());\
            CLOSE_THREAD(1);\
        }\
        bytes_sent += ret;\
    }\
} while (0)

void * connection_handler(void *args){
    int ret, recv_bytes, bytes_sent, err = 0;
    handler_args_t *arg = (handler_args_t *) args;
    /* The buffer to store the incoming messages */
    char buf[HEADER_L];
    size_t msg_len;

    new_user(G_UNKNOWN);

    char *word;
    struct message_s message;
    struct user_s user;

    memset(&message, 0, sizeof(struct message_s));
    memset(&user, 0, sizeof(struct user_s));
    /* Be sure the user is logged out at the begining */
    user.logged = OUT;

    struct sembuf oper[1];
    SEM_OPER(0, SEM_THREAD_ID, 1, 0);
    while (semop(sem_id, oper, 1) == -1) {
        if (server_error) CLOSE_THREAD(0);
        if (errno == EINTR) continue;
        LOG(LOG_ERR, "[%lu] Cannot operate on the thread semaphore.\n", pthread_self());
        LOG(LOG_ERR, "%s\n", strerror(errno));
        kill(getpid(), SIGUSR1);
        CLOSE_THREAD(1);
    }

    /* The buffer to store messages we send */
    char *msg_buf = malloc(MSG_BUF_LEN);
    if (msg_buf == NULL) {
        LOG(LOG_ERR, "[%lu] Cannot malloc to msg_buf.\n", pthread_self());
        FREE_RES();
        CLOSE_THREAD(1);
    }

    /* Parse client IP address and port */
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(arg->client_addr->sin_addr), client_ip, INET_ADDRSTRLEN);
    uint16_t client_port = ntohs(arg->client_addr->sin_port);

    LOG(LOG_INFO, "[%lu] New connection from %s on port: %d.\n",pthread_self(), client_ip, client_port);

    struct timeval timeout;
    timeout.tv_sec = SEC_TO_WIAT;
    timeout.tv_usec = NSEC_TO_WAIT;

    ret = setsockopt(arg->socket_desc, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval));
    if (ret < 0) {
        LOG(LOG_ERR, "[%lu] Cannot set timeout of the socket.\n", pthread_self());
        FREE_RES();
        CLOSE_THREAD(1);
    }
    ret = setsockopt(arg->socket_desc, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(struct timeval));
    if (ret < 0) {
        LOG(LOG_ERR, "[%lu] Cannot set timeout of the socket.\n", pthread_self());
        FREE_RES();
        CLOSE_THREAD(1);
    }
    /* The user stays in this loop */
    while (1) {
        recv_bytes = 0;
        msg_len = 0;
        do {
            if (server_error) {
                /* Some thread has found an error, so better finish it here */
                sprintf(msg_buf, INT_ERROR);
                SEND(msg_len, bytes_sent, ret);
                FREE_RES();
                CLOSE_THREAD(0);
            }
            /* Recive from the user */
            while ( (recv_bytes = recv(arg->socket_desc, buf + msg_len, HEADER_L - msg_len, 0)) <= 0 ) {
                if (recv_bytes == 0) {
                    LOG(LOG_ERR, "[%lu] Socket closed.\n", pthread_self());
                    FREE_RES();
                    CLOSE_THREAD(0);
                }
                if (errno == EINTR) continue;
                LOG(LOG_ERR, "[%lu] Cannot read from the socket.\n", pthread_self());
                FREE_RES();
                if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINPROGRESS) {
                    LOG(LOG_INFO, "[%lu] Socket timeoout.\n", pthread_self());
                    CLOSE_THREAD(0);
                }
                CLOSE_THREAD(1);
            }

            msg_len += recv_bytes;

            if (msg_len == HEADER_L) {
                /* Recived a full buffer, ther is no space for '\0' character */
                sprintf(msg_buf, BAD_REQUEST);
                SEND(msg_len, bytes_sent, ret);
                break;
            } else {
                /* Exit the loop il the '\n' was recived */
                if (strchr(buf, '\n') != NULL) break;
            }
        } while(1);

        buf[msg_len - 1] = '\0';
        /* Get the first word in the buffer */
        word = strtok(buf, " ");
        if (word == NULL) {
            /* No ' ' character was found in the buffer */
            sprintf(msg_buf, BAD_REQUEST);
        } else {
            /* Get the command */
            message.command = get_cmd(word);
            if (message.command == UNKNOWN) {
                /* Bad command */
                sprintf(msg_buf, BAD_CMD);
            } else {
                /* Proceed with the command */
                message.arg1 = strtok(NULL, " ");
                message.arg2 = strtok(NULL, " ");
                message.arg3 = strtok(NULL, " ");

                err = proccess_command(&message, &user, msg_buf, arg->socket_desc);
            }
        }

        SEND(msg_len, bytes_sent, ret);

        if (err == -1) {
            /* The worker found an error */
            FREE_RES();
            CLOSE_THREAD(1);
        }

        /* User out */
        if (message.command == ME_OUT || message.command == ME_NEW) break;
    }
    FREE_RES();
    CLOSE_THREAD(0);
}

int proccess_command(struct  message_s *message, struct user_s *user, char *msg_buf, const int sock){
    int ret = 0;
    struct sembuf oper[2];
    switch (message->command) {
        case ME_IS:
            if (user->logged == IN) {
                sprintf(msg_buf, NEED_LOGGOUT);
                break;
            }
            if (CHECK_ALNUM(message->arg1, USERNAME_L) &&
                CHECK_ALNUM(message->arg2, USER_PASS_L) &&
                message->arg3 == NULL) {
                SEM_OPER(0, SEM_USER_ID, -1, 0);
                SEM_DO(1, "user");
                ret = me_is(message, msg_buf);
                SEM_OPER(0, SEM_USER_ID, 1, 0);
                SEM_DO(1, "user");
                if (ret > 0) {
                    strcpy(user->name, message->arg1);
                    user->level = (ret == 'a' ? ADMIN : USER);
                    user->logged = IN;
                    LOG(LOG_INFO, "[%lu] User '%s' logged in.\n", pthread_self(), user->name);
                    new_user(user->level == ADMIN ? G_ADMIN : G_USER);
                }
            } else {
                sprintf(msg_buf, BAD_REQUEST);
            }
            break;
        case ME_OUT:
            if (user->logged) {
                LOG(LOG_INFO, "[%lu] User '%s' logged out.\n", pthread_self(), user->name);
                sprintf(msg_buf, "Your connection is now being destroyed. See you soon %s.\n\n", user->name);
            } else {
                message->command = ME_IS;
                sprintf(msg_buf, NEED_LOGGIN);
            }
            break;
        case ME_NEW:
            if (user->logged == IN) {
                message->command = ME_IS;
                sprintf(msg_buf, NEED_LOGGOUT);
                break;
            }
            if (CHECK_ALNUM(message->arg1, USERNAME_L) &&
                CHECK_ALNUM(message->arg2, USER_PASS_L) &&
                CHECK_LEVEL(message->arg3)) {
                LOG(LOG_INFO, "[%lu] New user request.\n", pthread_self());
                SEM_OPER(0, SEM_USER_ID, -1, 0);
                SEM_OPER(1, SEM_REQ_ID, -1, 0);
                SEM_DO(2, "user-request");
                ret = me_new(message, msg_buf);
                SEM_OPER(0, SEM_USER_ID, 1, 0);
                SEM_OPER(1, SEM_REQ_ID, 1, 0);
                SEM_DO(2, "user-request");
            } else {
                sprintf(msg_buf, BAD_REQUEST);
            }
            break;
        case ADD_THIS:
        case CANCEL:
        case UPDATE:
            if (user->logged == OUT) {
                sprintf(msg_buf, NEED_LOGGIN);
                break;
            }
            if (user->level == USER) {
                sprintf(msg_buf, NO_PERMISION);
                break;
            }
            if (CHECK_ALPHA(message->arg1, ENTRY_NAME_L) &&
                CHECK_ALPHA(message->arg2, ENTRY_LAST_L) &&
                CHECK_DIGIT(message->arg3, ENTRY_NUMBER_L)) {
                SEM_OPER(0, SEM_WRITE_ID, 0, 0);
                SEM_OPER(1, SEM_WRITE_ID, 1, 0);
                SEM_DO(2, "write");
                SEM_OPER(0, SEM_READ_ID, 0, 0);
                SEM_DO(1, "read");
                if (message->command == ADD_THIS) {
                    LOG(LOG_INFO, "[%lu] User '%s' adding an entry.\n", pthread_self(), user->name);
                    ret = add(message, msg_buf);
                } else if (message->command == CANCEL) {
                    LOG(LOG_INFO, "[%lu] User '%s' cancelling an entry.\n", pthread_self(), user->name);
                    ret = cancel(message, msg_buf);
                } else {
                    LOG(LOG_INFO, "[%lu] User '%s' updating an entry.\n", pthread_self(), user->name);
                    ret = update(message, msg_buf);
                }
                SEM_OPER(0, SEM_WRITE_ID, -1, 0);
                SEM_DO(1, "write")
            } else {
                sprintf(msg_buf, BAD_REQUEST);
            }
            break;
        case FIND_EXP:
            if (user->logged == OUT) {
                sprintf(msg_buf, NEED_LOGGIN);
                break;
            }
            if (CHECK_EXP_ALPHA(message->arg1, ENTRY_NAME_L) &&
                CHECK_EXP_ALPHA(message->arg2, ENTRY_LAST_L) &&
                CHECK_EXP_DIGIT(message->arg3, ENTRY_NUMBER_L)) {
                LOG(LOG_INFO, "[%lu] User '%s' making a search.\n", pthread_self(), user->name);
                SEM_OPER(0, SEM_WRITE_ID, 0, 0);
                SEM_OPER(1, SEM_READ_ID, 1, 0);
                SEM_DO(2, "write-read");
                ret = find_exp(sock, message, msg_buf);
                SEM_OPER(0, SEM_READ_ID, -1, 0);
                SEM_DO(1, "read");
            } else {
                sprintf(msg_buf, BAD_REQUEST);
            }
            break;
        default:
            sprintf(msg_buf, BAD_CMD);
    }
    return ret;
}

int get_cmd(const char *cmd){
    if (!strcmp(cmd, "ME_IS")) {
        return ME_IS;
    } else if (!strcmp(cmd, "ME_NEW")) {
        return ME_NEW;
    } else if (!strcmp(cmd, "ME_OUT")) {
        return ME_OUT;
    } else if (!strcmp(cmd, "UPDATE")) {
        return UPDATE;
    } else if (!strcmp(cmd, "CANCEL")) {
        return CANCEL;
    } else if (!strcmp(cmd, "FIND_EXP")) {
        return FIND_EXP;
    } else if (!strcmp(cmd, "ADD_THIS")) {
        return ADD_THIS;
    } else {
        return UNKNOWN;
    }
}

int check_alpha(char *string){
    do {
        if (!isalpha(*string)) {
            return 0;
        }
    } while (*(++string));
    return 1;
}

int check_digit(char *string){
    do {
        if (!isdigit(*string)) {
            return 0;
        }
    } while (*(++string));
    return 1;
}
int check_alnum(char *string){
    do {
        if (!isalnum(*string)) {
            return 0;
        }
    } while (*(++string));
    return 1;
}
