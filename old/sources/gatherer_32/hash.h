#include <string.h>

int hash(char*);

typedef union {
    int num;
    char str[4];
} h_set;

int hash(char* input) {
    int length = strlen(input);
    int hval = 0;
    h_set hset;
    hset.num = 0;
    int i, j;
    int remain = length % 4;
    for (i = 0; i < (length - remain); i += 4) {
        for (j = 0; j < 4; j++) {
            hset.str[j] = input[i + j];
        }
        hval ^= hset.num;
    }
    if (remain != 0) {
        h_set temp_set;
        temp_set.num = 0;
        for (j = 0; j < remain; j++) {
            temp_set.str[j] = input[i + j];
        }
        hval ^= temp_set.num;
    }
    return hval;
}
