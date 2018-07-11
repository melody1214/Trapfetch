#include "analyzer.h"

typedef struct {
    char path[512];
    long long timestamp;
    long long offset;
    long long length;
    long lba;
    int ino;
}entry;

typedef struct {
    int front;
    int rear;
    int count;
    entry ele[QUEUE_MAX];
}queue;

int is_full(queue q)
{
    int full = 0;

    if (q->count == QUEUE_MAX) {
        full = 1;
    }

    return full;
}
