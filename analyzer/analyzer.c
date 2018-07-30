#include "analyzer.h"
#include "queue.h"
#include "file.h"

FILE *fp_read;
FILE *fp_candidates;

queue q;
read_list *rl;
mm_list *ml;
pf_list *pl;

// an_init prepares to begin to analyze traced data.
// if it fails, returns true, if not, returns false.
bool an_init(char **argv)
{
    // open log files.
    fp_read = get_fd(argv[1], PATH_READ_LOG, OPEN_READ);
    fp_candidates = get_fd(argv[1], PATH_CANDIDATE_LOG, OPEN_READ);

    if (fp_read == NULL || fp_candidates == NULL)
    {
        return false;
    }

    // initialize read list.
    rl = init_read_list();
    // initialize mmap list.
    ml = init_mm_list();
    // initialize prefetch group list.
    pl = init_pf_list();
    // initialize queue.
    queue_init(&q);

    return true;
}

bool analyze()
{
    char buf[512];
    mm_node *m;
    read_node *r;
    struct stat st;
   
    bool is_last = false;

    // read line from read and candidate logs.
    if (!fgets(buf, sizeof(buf), fp_read))
    {
        // end of analysis.
        is_last = true;
        goto full;
    }

    if (buf[0] == 'm')
    {
        if (buf[2] == 'e')
        {
            // Create mmap node and insert that into mmap list.
            m = new_mmap_node(buf);
            m->md = gen_message_digest(m->path);
            insert_node_into_mm_list(ml, m);
        }

        // Create read node for prefetching and enqueue.
        r = new_read_node(buf, MMAP);
    }
    else if (buf[0] == 'r')
    {
        // Create read node for prefetching and enqueue.
        r = new_read_node(buf, READ);
    }

    // If a file has not logical block address, it will be not queued.
    r->lba = get_logical_blk_addr(r->path);
    if (r->lba == 0)
    {
        return true;
    }

    if (stat(r->path, &st) < 0)
    {
        perror("stat");
        return true;
    }

    r->ino = st.st_ino;
    enqueue(&q, r);

    // Queue is not full.
    if (!is_full(&q))
    {
        return true;
    }

full:
    // Queue is full. Queued data will be inserted to a read list.
    while (q.count > 0)
    {
        r = new_read_node(NULL, 0);
        memcpy(r, dequeue(&q), sizeof(read_node));
        insert_node_into_read_list(rl, r);
    }

    // Queue must have been reset.
    if (!is_empty(&q))
    {
        perror("is_empty()");
        exit(EXIT_FAILURE);
    }

    rl->start_ts = rl->head->ts;
    rl->end_ts = rl->tail->ts;

    // The time for fully queueing is less than burst threshold.
    if (is_burst(&q)) {
        rl->is_burst = IS_BURST;
    }
    // The time for fully queueing exceeds the threshold.
    else {
        rl->is_burst = !IS_BURST;
    }

    if (is_last) {
        pl->tail = rl;
        return false;
    }

    insert_read_list_into_pf_list(pl, rl);

    rl->next = init_read_list();
    rl = rl->next;

    return true;
}

void reordering_pf_list()
{
   read_list *pf = pl->head;
   read_list *read = pl->head;

   while (read != NULL) {
       if (read->next == NULL) {
           pl->tail = pf;
           return;
       }

       if (read->is_burst == IS_BURST) {
           if (read->next->is_burst != IS_BURST) {
               pf->end_ts = read->end_ts;
               pf->tail = read->tail;
           }
           else {
               read->tail->next = read->next->head;
               pf->end_ts = read->next->end_ts;
               pf->tail = read->next->tail;
           }
       }

       if (read->is_burst != IS_BURST) {
            if (read->next->is_burst == IS_BURST) {
                pf->next = read->next;
                pf = pf->next;
            }
       }

       read = read->next;
   }
}

