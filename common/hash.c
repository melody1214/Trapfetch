#include <string.h>

/*
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
*/

size_t fnv1a_hash(const char* cp)
{
    size_t hash = 0x811c9dc5;
    while (*cp) {
        hash ^= (unsigned char) *cp++;
        hash *= 0x01000193;
    }
    return hash;
}
