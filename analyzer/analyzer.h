#include <stdio.h>
#include <stdbool.h>

#define PATH_READ_LOG "/home/melody/study/projects/trapfetch/logs/r."
#define PATH_CANDIDATE_LOG "/home/melody/study/projects/trapfetch/logs/c."
#define OPEN_READ 0
#define OPEN_WRITE 1

#define IS_BURST 1

#define READ 1
#define MMAP 2

#define QUEUE_MAX 32
#define BURST_THRESHOLD 200000000

typedef struct _read_node
{
    char path[512];
    long long ts;
    long long off;
    long long len;
    long lba;
    int ino;
    struct _read_node *next;
} read_node;

typedef struct _read_list
{
    read_node *head;
    read_node *tail;
    long long start_ts;
    long long end_ts;
    bool is_burst;
    struct _read_list *next;
} read_list;

typedef struct _pf_list
{
    read_list *head;
    read_list *tail;
} pf_list;

typedef struct _mm_node
{
    char path[512];
    long long ts;
    long long off;
    long long len;
    unsigned long md;
    void *start_addr;
    void *end_addr;
    struct _mm_node *next;
} mm_node;

typedef struct _mm_list
{
    mm_node *head;
    mm_node *tail;
} mm_list;

typedef struct _queue
{
    int front;
    int rear;
    int count;
    read_node ele[QUEUE_MAX];
} queue;

read_list *init_read_list()
{
    read_list *newlist = malloc(sizeof(read_list));

    newlist->head = NULL;
    newlist->tail = NULL;
    newlist->next = NULL;

    return newlist;
}

mm_list *init_mm_list()
{
    mm_list *newlist = malloc(sizeof(mm_list));

    newlist->head = NULL;
    newlist->tail = NULL;

    return newlist;
}

pf_list *init_pf_list(read_list *rl)
{
    pf_list *newlist = malloc(sizeof(pf_list));

    newlist->head = rl;
    newlist->tail = rl;

    return newlist;
}

read_node *new_read_node(char *buf, int type)
{
    read_node *newnode;
    newnode = malloc(sizeof(read_node));

    switch (type)
    {
    case READ:
        sscanf(buf, "%*[^,],%[^,],%lld,%lld,%lld", newnode->path, newnode->ts, newnode->off, newnode->len);
        break;
    case MMAP:
        sscanf(buf, "%*[^,],%*[^,],%[^,],%lld,%lld,%lld,", newnode->path, newnode->ts, newnode->off, newnode->len);
        break;
    default:
        break;
    }

    newnode->next = NULL;

    return newnode;
}

mm_node *new_mmap_node(char *buf)
{
    mm_node *newnode;
    char fname[512];

    memset(fname, '\0', sizeof(fname));
    newnode = malloc(sizeof(mm_node));

    sscanf(buf, "%*[^,]%*[^,]%[^,],%lld,%lld,%lld,%p,%p", newnode->path, newnode->ts,
           newnode->off, newnode->len, newnode->start_addr, newnode->end_addr);

    newnode->next = NULL;

    return newnode;
}

void insert_node_into_mm_list(mm_list *l, mm_node *n)
{
    mm_node *tmp = l->head;

    if (tmp == NULL)
    {
        l->head = n;
        l->tail = n;
        return;
    }

    while (tmp != NULL)
    {
        if (tmp->next != NULL)
        {
            tmp = tmp->next;
            continue;
        }
        tmp->next = n;
    }

    l->tail = n;
}

void insert_node_into_read_list(read_list *rl, read_node *n)
{
    read_node *tmp = rl->head;

    if (tmp == NULL)
    {
        rl->head = n;
        rl->tail = n;
        return;
    }

    while (tmp != NULL)
    {
        if (tmp->next != NULL)
        {
            tmp = tmp->next;
            continue;
        }
        tmp->next = n;
    }

    rl->tail = n;
}

void insert_read_list_into_pf_list(pf_list *pl, read_list *rl)
{
    read_list *tmp = pl->head;

    if (tmp == NULL)
    {
        pl->head = rl;
        pl->tail = rl;
        return;
    }

    while (tmp != NULL)
    {
        if (tmp->next != NULL)
        {
            tmp = tmp->next;
            continue;
        }
        tmp->next = rl;
    }

    pl->tail = rl;
}