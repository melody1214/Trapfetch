#include "analyzer.h"

typedef struct {
    char path[512];
    long long ts;
    long long off;
    long long len;
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

        q->ele[i].ts = 0;
        q->ele[i].off = 0;
        q->ele[i].len = 0;
        q->ele[i].lba = 0;
        q->ele[i].ino = 0;

        i++;
    }  
}

void enqueue(queue *q, read_node *r)
{
    strcpy(q->ele[i].path, r->path);

    q->ele[i].ts = r->ts;
    q->ele[i].off = r->off;
    q->ele[i].len = r->len;
    q->ele[i].lba = r->lba;
    q->ele[i].ino = r->ino;

    q->count++;
    q->rear++;
}

bool is_full(queue *q)
{
    if (q->count == QUEUE_MAX) {
        return true;
    }

    return false;
}

bool is_burst(queue *q)
{
    entry rear = q->ele[q->rear];
    entry front = q->ele[q->front];

    if (rear.ts - front.ts < BURST_THRESHOLD) {
        return true;
    }

    return false;
}