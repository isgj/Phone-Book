#ifndef  LENGTHS_H
#define  LENGTHS_H

#define USERNAME_L 12
#define USER_PASS_L 16
#define HASH_L 32
#define ENTRY_NAME_L 12
#define ENTRY_LAST_L 12
#define ENTRY_NUMBER_L 10

#define USER_LINE_L (((USERNAME_L + HASH_L + 6) / 4) * 4)
#define ENTRY_LINE_L (((ENTRY_NAME_L + ENTRY_LAST_L + ENTRY_NUMBER_L + 6) / 4) * 4)

#define MAX(a, b) ((a)>(b)?(a):(b))
#define HEADER_L (MAX(USER_LINE_L, ENTRY_LINE_L))

#endif
