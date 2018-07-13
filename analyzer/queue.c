#include "analyzer.h"

typedef struct {
    char path[512];
    long long timestamp;
    long long offset;
    long long length;
    long lba;
    int ino;
}entry;

struct queue {
    int front;
    int rear;
    int count;
    entry ele[QUEUE_MAX];
};

void queue_init(queue *q)
{
    int i;

    q->front = 0;
    q->rear = 0;
    q->count = 0;

    i = 0;
    while (i < QUEUE_MAX) {
        memset(q->ele[i].path, '\0', sizeof(q->ele[i].path));

        q->ele[i].timestamp = 0;
        q->ele[i].offset = 0;
        q->ele[i].length = 0;
        q->ele[i].lba = 0;
        q->ele[i].ino = 0;

        i++;
    }  
}

void enqueue(queue *q)
{
    
}

int is_full(queue *q)
{
    int full = 0;

    if (q->count == QUEUE_MAX) {
        full = 1;
    }

    return full;
}

