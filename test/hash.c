#include <string.h>
#include <stdio.h>

typedef union {
    int num;
    char str[4];
} h_set;

static size_t djb_hash(const char* cp)
{
    size_t hash = 5381;
    while (*cp)
        hash = 33 * hash ^ (unsigned char) *cp++;
    return hash;
}

static size_t fnv1a_hash(const char* cp)
{
    size_t hash = 0x811c9dc5;
    while (*cp) {
        hash ^= (unsigned char) *cp++;
        hash *= 0x01000193;
    }
    return hash;
}


unsigned long hash(char *input) {
    /*
    char buf[512];
    int length;
    int hval;
    int i, j;
    int remain;
    h_set hset;

    memset(buf, '\0', 512 * sizeof(char));
    strncpy(buf, input, 512 * sizeof(char));

    hset.num = 0;
    length = strlen(buf);
    remain = length & 4;

    for (i = 0; i < (length - remain); i += 4) {
        for (j = 0; j < 4; j++) {
            hset.str[j] = buf[i + j];
        }
        hval ^= hset.num;
    }

    if (remain != 0) {
        h_set tmp;
        tmp.num = 0;
        for (j = 0; j < remain; j++) {
            tmp.str[j] = buf[i + j];
        }              
        hval ^= tmp.num;
    }

    return hval;
    */
    
    
    unsigned long hash = 5381;
    int c;
    
    while (c = *input++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    

    return hash;

    /*
    int hash = 7;
        for (int i = 0; i < strlen; i++) {
    hash = hash*31 + charAt(i);

    return hash;
    */
}

unsigned long gen_message_digest(char *input) {
    return hash(input);
}

int main() {
    char *str1 = "/home/melody/GOG Games/Divinity Original Sin Enhanced Edition/game/libicuuc.so.54";
    char str2[512];

    memset(str2, '\0', sizeof(str2));
    strncpy(str2, str1, sizeof(str2));

    printf("%ld\n", djb_hash(str1));
    printf("%ld\n", djb_hash(str1));
    printf("%ld\n", fnv1a_hash(str1));
    printf("%ld\n", fnv1a_hash(str1));   

    printf("%ld\n", gen_message_digest(str2));
    printf("%ld\n", gen_message_digest(str2));
    printf("%ld\n", gen_message_digest(str2));
    printf("%ld\n", gen_message_digest(str2));   

    return 0;
}