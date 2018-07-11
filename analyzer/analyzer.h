#define	PATH_READ_LOG	"/home/melody/study/projects/trapfetch/logs/r."
#define PATH_CANDIDATE_LOG "/home/melody/study/projects/trapfetch/logs/c."
#define	OPEN_READ	0
#define	OPEN_WRITE	1

typedef struct _read_node {
    char path[512];
    long long timestamp;
    long long offset;
    long long length;
    long lba;
    int ino;
    struct _read_node *next;
}

typedef struct _mm_node {
    char path[512];
    long long timestamp;
    long long offset;
    long long length;
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