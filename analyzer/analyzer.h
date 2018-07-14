#define	PATH_READ_LOG	"/home/melody/study/projects/trapfetch/logs/r."
#define PATH_CANDIDATE_LOG "/home/melody/study/projects/trapfetch/logs/c."
#define	OPEN_READ	0
#define	OPEN_WRITE	1

#define READ    0
#define MMAP    1

typedef struct _read_node {
    char path[512];
    long long timestamp;
    long long offset;
    long long length;
    long lba;
    int ino;
    struct _read_node *next;
}read_node;

typedef struct _mm_node {
    char path[512];
    long long ts;
    long long off;
    long long len;
    long lba;
    unsigned long md;
    int ino;
    void *start_addr;
    void *end_addr;
    struct _mm_node *next; 
}mm_node;

struct mm_list {
    mm_node *head;
    mm_node *tail;
};

read_node *new_read_node(char *buf, int type)
{
    read_node *newnode;
    char fname[512];

    memset(fname, '\0', sizeof(fname));
    newnode = malloc(sizeof(read_node));

    if (type == READ) {
        sscanf(buf, "%*[^,],%[^,],%lld,%lld,%lld", newnode->path, newnode->ts, newnode->off, newnode->len);
    }
    else {
        sscanf(buf, "%*[^,],%*[^,],%[^,],%lld,%lld,%lld,", newnode->path, newnode->ts, newnode->off, newnode->len);        
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

void insert_node_into_mm_list(struct mm_list *l, mm_node *n) {
    mm_node *tmp = l->head;

    if (tmp == NULL) {
        l->head = n;
        l->tail = n;
        return;
    }

    while (tmp != NULL) {
       if (tmp->next != NULL) {
           tmp = tmp->next;
           continue;
       } 
       tmp->next = n;
    }
}