#include "queue.h"

void queue_init(queue *q)
{
    int i;

    q->front = 0;
    q->rear = -1;
    q->count = 0;

    i = 0;
    while (i < QUEUE_MAX)
    {
        memset(q->ele[i].path, '\0', sizeof(q->ele[i].path));

        q->ele[i].ts = 0;
        q->ele[i].off = 0;
        q->ele[i].len = 0;
        q->ele[i].lba = 0;
        q->ele[i].ino = 0;
        q->ele[i].next = NULL;

        i++;
    }
}

void enqueue(queue *q, read_node *r)
{
    q->rear = (q->rear + 1) % QUEUE_MAX;

    // checkpoint
    memcpy(&q->ele[q->rear], r, sizeof(read_node));

    q->count++;
    // q->ele[i].ts = r->ts;
    // q->ele[i].off = r->off;
    // q->ele[i].len = r->len;
    // q->ele[i].lba = r->lba;
    // q->ele[i].ino = r->ino;
}

read_node *dequeue(queue *q)
{
    read_node *ret = &q->ele[q->front];

    q->count--;
    q->front = (q->front + 1) % QUEUE_MAX;

    return ret;
}

bool is_empty(queue *q)
{
    if (q->count == 0)
    {
        return true;
    }

    return false;
}

bool is_full(queue *q)
{
    if (q->count == QUEUE_MAX)
    {
        return true;
    }

    return false;
}

bool is_burst(queue *q)
{
    read_node *rear = &q->ele[q->rear];
    read_node *front = &q->ele[q->front];

    if ((rear->ts - front->ts) < BURST_THRESHOLD)
    {
        return true;
    }

    return false;
}