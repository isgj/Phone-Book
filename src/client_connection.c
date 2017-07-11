#include "../include/client_connection.h"
#include "../include/client_menu.h"
#include "../include/lengths.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

extern int sock_fd;
extern int server_port;
extern char *server_ip;
extern int not_connected;
extern char *user_name;
extern char *password;

int connect_me(){
    int try = 0;

    STATUS(COLOR_PAIR(1), "Not connected.");
    // create a socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        end_win();
        fprintf(stderr, "Couldn't create the socket\n");
        exit(0);
    }

    struct sockaddr_in server_addr = {0}; 
    // set up parameters for the connection
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(server_port);

    while (try <= TRY_NUMBER) {
        STATUS(COLOR_PAIR(2), "Trying to establish a new connection.");

        alarm(5);
        add_to_swindow(3, "Trying to connect... ");
        // initiate a connection on the socket
        int ret = connect(sock_fd, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
        if (ret < 0) {
            ++try;
            STATUS(COLOR_PAIR(1), "Connection error.");
            if (errno != EINTR) add_to_swindow(1, "%s\n", strerror(errno));
            alarm(0);
            continue;
        }
        alarm(0);
        try = 0;
        break;
    }

    if (!try){
        STATUS(COLOR_PAIR(2), "%s", "Connection established!\n");
        add_to_swindow(2, "ok.\n");
        if (strlen(user_name)) {
            add_to_swindow(3, "Reconnecting as %s.\n", user_name);
            loggin(user_name, password);
        }
        return 0;
    }
    close(sock_fd);
    return -1;
}

int loggin(const char *name, const char *pass){

    char msg_buf[MSG_BUF_LEN];
    size_t msg_len;
    int bytes_sent, ret;

    sprintf(msg_buf, "ME_IS %s %s\n", name, pass);
    // send message to server
    SEND(msg_buf, msg_len, bytes_sent, ret);

    char level;
    int first_recv = 0, first_n = 0, prior, bytes_recv = 0;
    do {
        while ( (bytes_recv = recv(sock_fd, msg_buf, MSG_BUF_LEN - 1, 0)) <= 0 ) {
            if (errno == EINTR) continue;
            STATUS(COLOR_PAIR(1), "Connection error.");
            if (bytes_recv == 0) add_to_swindow(3, "\nThe server closed the connection.\n");
            if (bytes_recv < 0) add_to_swindow(1, "\nThere was an error while reciving the response.\n");
            close(sock_fd);
            return 1;
        }
        msg_buf[bytes_recv] = '\0';
        if (!first_recv) {
            first_recv = 1;
            level = msg_buf[0];
            prior = (level == 'u' || level == 'a') ? 2 : 1;
            if (bytes_recv > 2 ) {
                first_n = msg_buf[bytes_recv - 1] == '\n' ? 1 : 0;
                if (first_n && msg_buf[bytes_recv - 2] == '\n') {
                    msg_buf[bytes_recv - 1] = '\0';
                    add_to_swindow(prior, "%s", msg_buf + 2);
                    if (level == 'n') {
                        first_recv = 0;
                        return 1;
                    }
                    break;
                }
                add_to_swindow(prior, "%s", msg_buf + 2);
            }
        } else {
            if (first_n && msg_buf[0] == '\n') break;
            else {
                first_n = msg_buf[bytes_recv - 1] == '\n' ? 1 : 0;
                if (first_n && msg_buf[bytes_recv - 2] == '\n') {
                    msg_buf[bytes_recv - 1] = '\0';
                    add_to_swindow(prior, "%s", msg_buf);
                    if (level == 'n') {
                        first_recv = 0;
                        return 1;
                    }
                    break;
                }
                add_to_swindow(prior, "%s", msg_buf);
            }
        }
    } while(1);

    return level;
}

int send_get(const char *msg_buf){
    int ret, bytes_sent, msg_len;
    char test[16];
    ret = recv(sock_fd, test, 16, MSG_DONTWAIT);
    if (ret <= 0 ){
        if (!strcmp(msg_buf, "ME_OUT\n")) {
            add_to_swindow(2, "You are now log out.\n");
            return 0;
        }
        /* check errno if should reconnect */
        if (ret == 0 || (errno != EAGAIN && errno != EWOULDBLOCK)){
            ret = connect_me();
            /* no connection */
            if (ret < 0) return ret;
        }

    }
    SEND(msg_buf, msg_len, bytes_sent, ret);

    char msg[MSG_BUF_LEN];
    int bytes_recv, first_n = 0;
    RECV(msg, bytes_recv, first_n);
    return 0;
}

void sig_pipe(int sig) {
    STATUS(COLOR_PAIR(1), "Connection closed.");
    close(sock_fd);
}

void sig_handler(int sig) {
    end_win();
    fprintf(stderr, "Got signal %s.\n", sys_siglist[sig]);
    exit(0);
}

void sig_alarm(int sig) {
    add_to_swindow(1, "Connection timeout.\n");
}
