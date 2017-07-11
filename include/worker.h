#ifndef WORKER_H
#define WORKER_H


//#include <sys/types.h>


enum commands {UNKNOWN, ME_IS, ME_NEW, ME_OUT, ADD_THIS, UPDATE, CANCEL, FIND_EXP};
enum levels {USER, ADMIN};
enum status {OUT, IN};

struct user_s {
    char name[12];
    enum levels level;
    enum status logged;
};

struct message_s {
    enum commands command;
    char *arg1;
    char *arg2;
    char *arg3;
};

#define MSG_BUF_LEN 256

#define USER_FILE "users.txt"
#define REQ_FILE  "requests.txt"
#define BOOK_FILE "book.txt"

#define INT_ERROR "Internal error!!! Please contact the server administrator.\n\n"

#define MATCH_THIS(exp, pos) (!strncmp(exp, pos, strlen(exp)) && (*(pos + strlen(exp)) == ' ' || *(pos + strlen(exp)) == '\n'))
#define MATCH_EXP(exp, pos) (*exp == '*' || MATCH_THIS(exp, pos))

#define OPEN_MY_FILE(file_pointer, file_name, mode) do {\
    file_pointer = fopen(file_name, mode);\
    if (file_pointer == NULL) {\
        WORKER_ERROR("[%lu] Error opening file '%s' with mode '%s'.\n", pthread_self(), file_name, mode);\
    }\
} while (0)

#define SEND_MATCH(msg_len, bytes_sent, ret) do {\
    msg_len = strlen(msg_buf);\
    bytes_sent = 0;\
    while (bytes_sent < msg_len) {\
        ret = send(sock, msg_buf + bytes_sent, msg_len - bytes_sent, 0);\
        if (ret == -1 && errno == EINTR) continue;\
        if (ret < 0) WORKER_ERROR("[%lu] Error sending to the socket.\n", pthread_self());\
        bytes_sent += ret;\
    }\
} while (0)

#define CLOSE_MY_FILE(file, name) do {\
    while (fclose(file) == EOF) {\
        if (errno != EINTR) {\
            WORKER_ERROR("[%lu] Error closing file '%s'.\n", pthread_self(), name);\
        }\
    }\
} while (0)

#define CHECK_ERROR(file, name) do {\
    if (ferror(file)) {\
        WORKER_ERROR("[%lu] Error reading file '%s'.\n", pthread_self(), name);\
    }\
} while (0)

#define GET_HASH() \
    char hash_code[HASH_L];\
    unsigned char hash[MD5_DIGEST_LENGTH];\
    MD5((const unsigned char *)message->arg2, strlen(message->arg2), hash);\
    for(int i = 0; i < MD5_DIGEST_LENGTH; i++) {\
        sprintf(&hash_code[i*2], "%02x\n", hash[i]);\
    }\

int me_is(const struct message_s *message, char *msg_buf);
int me_new(const struct message_s *message, char *msg_buf);
int find_exp(int sock, const struct message_s *message, char *msg_buf);
int add(const struct message_s *message, char *msg_buf);
int cancel(const struct message_s *message, char *msg_buf);
int update(const struct message_s *message, char *msg_buf);
int copy_new_size(FILE *file, char *msg_buf, const char *new_entry, const char *file_name);
int get_a_request(char *msg);
int promote_first_req(char *msg_buf, int promote);
int req_number(void);

#endif
