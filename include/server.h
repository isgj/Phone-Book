#ifndef SERVER_H
#define SERVER_H

#define WAIT_TIME_SEC 1

int server_error = 0;
int my_t_total = 0;
int my_u_logged = 0;
int my_a_logged = 0;

#define STAT_FILE(file, ret)    do {\
    memset(&st, 0, sizeof(struct stat));\
    stat(file, &st);\
    if (S_ISREG(st.st_mode) != 1) {\
        ret = creat(file, 0666);\
        ERROR_HELPER(ret, "Couldn't creat file: '" file "'.\n");\
        close(ret);\
    }\
} while (0)

#endif
