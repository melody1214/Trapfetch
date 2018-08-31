#include <string.h>
#include <stdio.h>

typedef union {
    int num;
    char str[4];
} h_set;

int hash(char *input) {
    int length;
    int hval;
    int i, j;
    int remain;
    h_set hset;

    hset.num = 0;
    length = strlen(input);
    remain = length & 4;

    for (i = 0; i < (length - remain); i += 4) {
        for (j = 0; j < 4; j++) {
            hset.str[j] = input[i + j];
        }
        hval ^= hset.num;
    }

    if (remain != 0) {
        h_set tmp;
        tmp.num = 0;
        for (j = 0; j < remain; j++) {
            tmp.str[j] = input[i + j];
        }              
        hval ^= tmp.num;
    }

    return hval;
}

int gen_message_digest(char *input) {
    return hash(input);
}

int main() {
    char *str1 = "/home/melody/GOG Games/Pillars of Eternity/game/PillarsOfEternity";
    char str2[512];

    memset(str2, '\0', sizeof(str2));
    strncpy(str2, str1, sizeof(str2));

    printf("%ld\n", (long)gen_message_digest(str2));   

    return 0;
}