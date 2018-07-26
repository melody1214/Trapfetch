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
    pl = init_pf_list();
    // initialize queue.
    queue_init(&q);

    return true;
}

bool analyze()
{
    char buf[512];
    mm_node *mnode;
    read_node *rnode;
    struct stat st;

    // read line from read and candidate logs.
    if (!fgets(buf, sizeof(buf), fp_read))
    {
        // end of analysis.
        return false;
    }

    if ((buf[0] == 'm') && (buf[2] == 'e'))
    {
        // Create mmap node and insert that into mmap list.
        mnode = new_mmap_node(buf);
        mnode->md = gen_message_digest(mnode->path);
        insert_node_into_mm_list(ml, mnode);

        // Create read node for prefetching and enqueue.
        rnode = new_read_node(buf, MMAP);
    }
    else if (buf[0] == 'r')
    {
        // Create read node for prefetching and enqueue.
        rnode = new_read_node(buf, READ);
    }

    rnode->lba = get_logical_blk_addr(rnode->path);

    // If a file has not logical block address, it will be not queued.
    if (rnode->lba == 0)
    {
        exit(EXIT_FAILURE);
    }

    if (stat(rnode->path, &st) < 0)
    {
        free(mnode);
        free(rnode);
        return true;
    }

    rnode->ino = st.st_ino;
    enqueue(&q, rnode);

    // Queue is not full.
    if (!is_full())
    {
        return true;
    }

    // The time for fully queueing is less than burst threshold.
    if (is_burst())
    {

        append_to_pf_group_list(&q, pl, IS_BURST);
    }
}