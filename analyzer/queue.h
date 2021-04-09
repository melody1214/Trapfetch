#ifndef _QUEUE_HEADER_INCLUDED
#define _QUEUE_HEADER_INCLUDED

#include <string.h>
#include <stdbool.h>

#ifndef _ANALYZER_HEADER_INCLUDED
#define _ANALYZER_HEADER_INCLUDED

#include "analyzer.h"

#endif

#define QUEUE_MAX 32
#define BURST_THRESHOLD 2500000000

typedef struct _queue
{
    int front;
    int rear;
    int count;
    read_node ele[QUEUE_MAX];
} queue;

void queue_init(queue *q);
void enqueue(queue *q, read_node *r);
read_node *dequeue(queue *q);
bool is_empty(queue *q);
bool is_full(queue *q);
bool is_burst(queue *q);
double get_queueing_time(queue *q);

#endif /* _QUEUE_HEADER_INCLUDED */
