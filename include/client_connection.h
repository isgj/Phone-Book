#ifndef CONNECTION_CLIENT_H
#define CONNECTION_CLIENT_H

#define MSG_BUF_LEN 256
#define TRY_NUMBER 5

#define ERROR_CONN(ret, msg) do {\
    if (ret < 0) {\
        add_to_swindow(1, "%s", msg);\
        return 1;\
    }\
} while (0)

#define SEND(msg_buf, msg_len, bytes_sent, ret) do {\
    msg_len = strlen(msg_buf);\
    bytes_sent = 0;\
    while (bytes_sent < msg_len) {\
        ret = send(sock_fd, msg_buf + bytes_sent, msg_len - bytes_sent, 0);\
        if (ret == -1 && errno == EINTR) continue;\
        if (ret < 0) {\
            if (!strcmp(msg_buf, "ME_OUT\n")) {\
                add_to_swindow(2, "You are now log out.\n");\
                return 0;\
            } else {\
                if (!connect_me()) {\
                    bytes_sent = 0;\
                    continue;\
                } else return 1;\
            }\
        }\
        bytes_sent += ret;\
    }\
} while (0)

#define RECV(msg_buf, bytes_recv, first_n) do {\
    while ( (bytes_recv = recv(sock_fd, msg_buf, MSG_BUF_LEN - 1, 0)) <= 0 ) {\
        if (bytes_recv == -1 && errno == EINTR) continue;\
        if (bytes_recv == 0) add_to_swindow(3, "\nThe server closed the connection.\n");\
        if (bytes_recv < 0) add_to_swindow(1, "\nThere was an error while reciving the response.\n");\
        close(sock_fd);\
        return 1;\
    }\
    msg_buf[bytes_recv] = '\0';\
    if (first_n && msg_buf[0] == '\n') break;\
    else {\
        first_n = msg_buf[bytes_recv - 1] == '\n' ? 1 : 0;\
        if (first_n && msg_buf[bytes_recv - 2] == '\n') {\
            msg_buf[bytes_recv - 1] = '\0';\
            add_to_swindow(2, "%s", msg_buf);\
            break;\
        }\
        add_to_swindow(2, "%s", msg_buf);\
    }\
} while(1);

int connect_me(void);
int loggin(const char *name, const char *pass);
int send_get(const char *msg_buf);
void sig_pipe(int sig);
void sig_handler(int sig);
void sig_alarm(int sig);
#endif
