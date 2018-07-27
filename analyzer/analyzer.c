#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include "analyzer.h"

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
    pl = init_pf_list(rl);
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

    // read line from read and candidate logs.
    if (!fgets(buf, sizeof(buf), fp_read))
    {
        // end of analysis.
        return false;
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
        free(m);
        free(r);
        exit(EXIT_FAILURE);
    }

    if (stat(r->path, &st) < 0)
    {
        free(m);
        free(r);
        perror("stat");
        exit(EXIT_FAILURE);
    }

    r->ino = st.st_ino;
    enqueue(&q, r);

    // Queue is not full.
    if (!is_full())
    {
        return true;
    }

    // The time for fully queueing is less than burst threshold.
    if (is_burst())
    {
        while (q.count > 0)
        {
            r = new_read_node(NULL, 0);
            memcpy(r, dequeue(q), sizeof(read_node));
            insert_read_node_into_read_list(rl, r);
        }

        if (!is_empty())
        {
            perror("is_empty()");
            exit(EXIT_FAILURE);
        }

        rl->start_ts = rl->head->ts;
        rl->end_ts = rl->tail->ts;
        rl->is_burst = IS_BURST;

        insert_read_list_into_pf_list(pl, rl);

        rl = init_read_list();
    }
}