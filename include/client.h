#ifndef CLIENT_H
#define CLIENT_H

#include <errno.h>
#include <string.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9090

#define GENERIC_ERROR_HELPER(cond, errCode, msg) do {\
    if (cond) {\
        fprintf(stderr, "%s", msg);\
        fprintf(stderr, "%s\n", strerror(errCode));\
        exit(EXIT_FAILURE);\
    }\
} while(0)

#define ERROR_HELPER(ret, msg)          GENERIC_ERROR_HELPER((ret < 0), errno, msg)
#define PTHREAD_ERROR_HELPER(ret, msg)  GENERIC_ERROR_HELPER((ret != 0), ret, msg)
#endif
