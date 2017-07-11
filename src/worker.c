#include <sys/mman.h>
#include <openssl/md5.h>


#include "../include/err_ex_util.h"
#include "../include/worker.h"
#include "../include/lengths.h"

int me_is(const struct message_s *message, char *msg_buf) {
    FILE *file;
    OPEN_MY_FILE(file, USER_FILE, "r");

    GET_HASH();

    char line[USER_LINE_L];
    int name_len = strlen(message->arg1);
    while (1) {
        char *input = fgets(line, USER_LINE_L, file);
        if (input == NULL && errno == EINTR) continue;
        if (input == NULL) break;
        if (!strncmp(line, message->arg1, name_len) && !(strncmp(line + name_len + 1, hash_code, HASH_L))) {
            CLOSE_MY_FILE(file, USER_FILE);
            sprintf(msg_buf, "%c Hello %s. You are now logged in.\n\n",line[strlen(line) - 2], message->arg1);
            return line[name_len + HASH_L + 2];
        }
    }

    CHECK_ERROR(file, USER_FILE);
    CLOSE_MY_FILE(file, USER_FILE);

    sprintf(msg_buf, "n Authentication failed. Plese double check your username and passwarod.\n\n");
    return 0;
}

int me_new(const struct message_s *message, char *msg_buf) {
    FILE *file;
    OPEN_MY_FILE(file, USER_FILE, "r");

    int name_len = strlen(message->arg1);

    while (1) {
        char *input = fgets(msg_buf, USER_LINE_L, file);
        if (input == NULL && errno == EINTR) continue;
        if (input == NULL) break;
        if (!strncmp(msg_buf, message->arg1, name_len)) {
            CLOSE_MY_FILE(file, USER_FILE);
            sprintf(msg_buf, "Username '%s' alredy exists. Try with another username.\n\n", message->arg1);
            return 0;
        }
    }

    CHECK_ERROR(file, USER_FILE);
    CLOSE_MY_FILE(file, USER_FILE);

    OPEN_MY_FILE(file, REQ_FILE, "r+");
    while (1) {
        char *input = fgets(msg_buf, USER_LINE_L, file);
        if (input == NULL && errno == EINTR) continue;
        if (input == NULL) break;
        if (!strncmp(msg_buf, message->arg1, name_len)) {
            CLOSE_MY_FILE(file, REQ_FILE);
            sprintf(msg_buf, "Username '%s' is in the request list. Please wait for the admin to aprove it.\n\n", message->arg1);
            return 0;
        }
    }

    CHECK_ERROR(file, REQ_FILE);
    GET_HASH()

    sprintf(msg_buf, "%s %34s\n", message->arg1, message->arg3);
    memcpy(msg_buf + name_len + 1, hash_code, HASH_L);
    while (fputs(msg_buf, file) == EOF) {
        if (errno == EINTR) continue;
        WORKER_ERROR("[%lu] Error writing file '%s'.\n", pthread_self(), REQ_FILE);
    }
    CLOSE_MY_FILE(file, REQ_FILE);

    line_num++;
    set_line_num();
    sprintf(msg_buf, "Your request was saved. See you soon.\n\n");
    return 0;
}

int find_exp(int sock, const struct message_s *message, char *msg_buf) {
    FILE *file;
    OPEN_MY_FILE(file, BOOK_FILE, "r");

    int ret, msg_len, bytes_sent, matches = 0;
    while (1) {
        char *input = fgets(msg_buf, ENTRY_LINE_L, file);
        if (input == NULL && errno == EINTR) continue;
        if (input == NULL) break;
        if (MATCH_EXP(message->arg1, msg_buf) &&
            MATCH_EXP(message->arg2, strchr(msg_buf, ' ') + 1) &&
            MATCH_EXP(message->arg3, strrchr(msg_buf, ' ') + 1)) {
            if (!matches) {
                msg_len = strlen(msg_buf);
                char line[msg_len + 1];
                memcpy(line, msg_buf, msg_len + 1);
                sprintf(msg_buf, "Here are the matched entries:\n");
                SEND_MATCH(msg_len, bytes_sent, ret);
                memcpy(msg_buf, line, msg_len + 1);
            }
            matches++;
            SEND_MATCH(msg_len, bytes_sent, ret);
        }
    }

    CHECK_ERROR(file, BOOK_FILE);
    CLOSE_MY_FILE(file, BOOK_FILE);

    if (!matches) {
        sprintf(msg_buf, "No entries matched your query.\n\n");
    } else {
        sprintf(msg_buf, "Number of entries : %d.\n\n", matches);
    }

    return 0;
}

int add(const struct message_s *message, char *msg_buf) {
    FILE *file;
    OPEN_MY_FILE(file, BOOK_FILE, "r+");

    char entry[ENTRY_LINE_L];
    sprintf(entry, "%s %s %s\n", message->arg1, message->arg2, message->arg3);

    while (1) {
        char *input = fgets(msg_buf, ENTRY_LINE_L, file);
        if (input == NULL && errno == EINTR) continue;
        if (input == NULL) break;
        if (!strncmp(msg_buf, entry, strlen(message->arg1) + strlen(message->arg2) + 2)) {
            sprintf(msg_buf, "The entry '%s %s' already exists in the book.\n\n",
                    message->arg1, message->arg2);
            CLOSE_MY_FILE(file, BOOK_FILE);
            return 0;
        }
    }
    CHECK_ERROR(file, BOOK_FILE);

    while (fputs(entry, file) == EOF) {
        if (errno == EINTR) continue;
        WORKER_ERROR("[%lu] Error writing file '%s'.\n", pthread_self(), BOOK_FILE);
    }

    sprintf(msg_buf, "The entry '%s %s' was succesfully added.\n\n",
            message->arg1, message->arg2);

    CLOSE_MY_FILE(file, BOOK_FILE);

    return 0;
}

int cancel(const struct message_s *message, char *msg_buf) {
    FILE *file;
    OPEN_MY_FILE(file, BOOK_FILE, "r+");

    int ret = 0, deleted = 0;
    while (1) {
        char *input = fgets(msg_buf, ENTRY_LINE_L, file);
        if (input == NULL && errno == EINTR) continue;
        if (input == NULL) break;
        if (MATCH_THIS(message->arg1, msg_buf) &&
            MATCH_THIS(message->arg2, strchr(msg_buf, ' ') + 1) &&
            MATCH_EXP(message->arg3, strrchr(msg_buf, ' ') + 1)) {
            deleted = 1;
            ret = copy_new_size(file, msg_buf, NULL, BOOK_FILE);
            break;
        }
    }

    if (ret != -1) {
        if (deleted) {
            sprintf(msg_buf, "The entry '%s %s' was deleted.\n\n", message->arg1, message->arg2);
        } else {
            CHECK_ERROR(file, BOOK_FILE);
            CLOSE_MY_FILE(file, BOOK_FILE);
            sprintf(msg_buf, "The entry '%s %s' was not found.\n\n", message->arg1, message->arg2);
        }
    }
    return ret;
}

int update(const struct message_s *message, char *msg_buf) {
    FILE *file;
    OPEN_MY_FILE(file, BOOK_FILE, "r+");

    int ret = 0, updated = 0;
    while (1) {
        char *input = fgets(msg_buf, ENTRY_LINE_L, file);
        if (input == NULL && errno == EINTR) continue;
        if (input == NULL) break;
        if (MATCH_THIS(message->arg1, msg_buf) &&
            MATCH_THIS(message->arg2, strchr(msg_buf, ' ') + 1)) {
            char new_entry[ENTRY_LINE_L];
            sprintf(new_entry, "%s %s %s\n", message->arg1, message->arg2, message->arg3);
            updated = 1;
            if (strlen(msg_buf) == strlen(new_entry)) {
                while (fseek(file, - strlen(new_entry), SEEK_CUR) == -1) {
                    if (errno == EINTR) continue;
                    WORKER_ERROR("[%lu] Error using fseek in file '%s'.\n", pthread_self(), BOOK_FILE);
                }
                while (fputs(new_entry, file) == EOF) {
                    if (errno == EINTR) continue;
                    WORKER_ERROR("[%lu] Error writing file '%s'.\n", pthread_self(), BOOK_FILE);
                }
                CLOSE_MY_FILE(file, BOOK_FILE);
            } else {
                ret = copy_new_size(file, msg_buf, new_entry, BOOK_FILE);
            }
            break;
        }
    }

    if (ret != -1) {
        if (updated) {
            sprintf(msg_buf, "The entry '%s %s' was updated.\n\n", message->arg1, message->arg2);
        } else {
            CHECK_ERROR(file, BOOK_FILE);
            CLOSE_MY_FILE(file, BOOK_FILE);
            sprintf(msg_buf, "The entry '%s %s' was not found.\n\n", message->arg1, message->arg2);
        }
    }
    return ret;
}

int copy_new_size(FILE *file, char *msg_buf, const char *new_entry, const char *file_name) {
    int end;
    while ((end = ftell(file)) == -1) {
        if (errno == EINTR) continue;
        WORKER_ERROR("[%lu] Error using ftell in file '%s'.\n", pthread_self(), file_name);
    }
    int start = end - strlen(msg_buf);

    struct stat sb = {0};
    int file_ds = fileno(file);
    if (fstat(file_ds, &sb) == -1) {
        WORKER_ERROR("[%lu] Error getting stats of file '%s'.\n", pthread_self(), file_name);
    }


    int len = new_entry != NULL ? strlen(new_entry) : 0;
    int new_size = sb.st_size - end + start + len;
    char *src;

    if (new_size > sb.st_size) {
        if (ftruncate(file_ds, new_size) == -1){
            WORKER_ERROR("[%lu] Error truncating file '%s'.\n", pthread_self(), file_name);
        }

        src = mmap(NULL, new_size, PROT_READ | PROT_WRITE, MAP_SHARED, file_ds, 0);
        if (src == MAP_FAILED) WORKER_ERROR("[%lu] Error maping the file.\n", pthread_self());
        memmove(src + start + len, src + end, sb.st_size - end);
        memcpy(src + start, new_entry, len);
        if (munmap(src, sb.st_size) == -1) WORKER_ERROR("[%lu] Error unmaping the file.\n", pthread_self());
    } else {
        src = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, file_ds, 0);
        if (src == MAP_FAILED) WORKER_ERROR("[%lu] Error maping the file.\n", pthread_self());
        memmove(src + start + len, src + end, sb.st_size - end);
        if (len) memcpy(src + start, new_entry, len);
        if (munmap(src, sb.st_size) == -1) WORKER_ERROR("[%lu] Error unmaping the file.\n", pthread_self());

        if (ftruncate(file_ds, new_size) == -1){
            WORKER_ERROR("[%lu] Error truncating file '%s'.\n", pthread_self(), file_name);
        }
    }
    clearerr(file);
    CLOSE_MY_FILE(file, file_name);

    return 0;
}

int get_a_request(char *msg_buf){
    FILE *file;
    OPEN_MY_FILE(file, REQ_FILE, "r");
    while (1) {
        char *line = fgets(msg_buf, USER_LINE_L, file);
        if (line == NULL && errno == EINTR) continue;
        break;
    }
    CHECK_ERROR(file, REQ_FILE);
    CLOSE_MY_FILE(file, REQ_FILE);

    return 0;
}

int promote_first_req(char *msg_buf, int promote){
    FILE *file;

    if (promote) {
        /* First add the user to the USER_FILE */
        OPEN_MY_FILE(file, USER_FILE, "a");
        while (fputs(msg_buf, file) == EOF) {
            if (errno == EINTR) continue;
            WORKER_ERROR("[%lu] Error writing file '%s'.\n", pthread_self(), BOOK_FILE);
        }
        CLOSE_MY_FILE(file, USER_FILE);
    }

    /* After cancel it from the REQ_FILE */
    OPEN_MY_FILE(file, REQ_FILE, "r+");
    while (1) {
        char *line = fgets(msg_buf, USER_LINE_L, file);
        if (line == NULL && errno == EINTR) continue;
        break;
    }
    CHECK_ERROR(file, REQ_FILE);

    return copy_new_size(file, msg_buf, NULL, REQ_FILE);
}

int req_number(){
    int lines = 0;
    FILE *file = fopen(REQ_FILE, "r");
    if (file == NULL) ERROR_HELPER(-1, "Cann't open the request file to get the number of the lines.\n");
    while (1) {
        int c = fgetc(file);
        if (c == EOF && errno == EINTR) continue;
        if (c == EOF) break;
        if (c == '\n') lines++;
    }
    if (!feof(file)) ERROR_HELPER(-1, "Found error getting the number of the lines.\n");
    while (fclose(file) == EOF) {
        if (errno != EINTR) ERROR_HELPER(-1, "Found error closing the file while getting the number of the lines.\n");
    }
    return lines;
}
