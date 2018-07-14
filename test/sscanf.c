#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct _read_node {
    char path[512];
    long long ts;
    long long off;
    long long len;
    long lba;
    int ino;
}read_node;

typedef struct _mm_node {
    char path[512];
    long long ts;
    long long off;
    long long len;
    unsigned long md;
    int ino;
    void *start_addr;
    void *end_addr;
}mm_node;

int main() {
    read_node *rnode = malloc(sizeof(read_node));
    mm_node *mnode = malloc(sizeof(mm_node));

    memset(mnode->path, '\0', sizeof(mnode->path));
    memset(rnode->path, '\0', sizeof(rnode->path));

    char *mlog = "m,e,/home/melody/study/projects/trapfetch/wrapper_64.so,6115893676081,0,2105536,242179387,0x7fa6f8216000,0x7fa6f84180c0\n";
    char *rlog = "r,/home/melody/GOG Games/Pillars of Eternity/game/PillarsOfEternity_Data/mainData,6116125486227,57344,7168\n";

    sscanf(mlog, "%*[^,],%*[^,],%[^,],%lld,%lld,%lld,%*[^,],%p,%p", mnode->path, &mnode->ts, 
    &mnode->off, &mnode->len, &mnode->start_addr, &mnode->end_addr);
    sscanf(rlog, "%*[^,],%[^,],%lld,%lld,%lld", rnode->path, &rnode->ts, &rnode->off, &rnode->len);

    printf("%s", mlog);
    printf("m,e,%s,%lld,%lld,%lld,242179387,%p,%p\n", mnode->path, mnode->ts, mnode->off, mnode->len, mnode->start_addr, mnode->end_addr);

    printf("%s", rlog);
    printf("r,%s,%lld,%lld,%lld\n", rnode->path, rnode->ts, rnode->off, rnode->len);

    free(rnode);
    free(mnode);

    return 0;       
}