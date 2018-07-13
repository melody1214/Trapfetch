#include "analyzer.h"

FILE *fp_read;
FILE *fp_candidates;

struct queue q;
struct mm_list *m;

// an_init prepares to begin to analyze traced data.
// if it fails, returns true, if not, returns false.
bool an_init(char **argv)
{
    // open log files.
    fp_read = get_fd(argv[1], PATH_READ_LOG, OPEN_READ);
    fp_candidates = get_fd(argv[1], PATH_CANDIDATE_LOG, OPEN_READ);

    if (fp_read == NULL || fp_candidates == NULL) {
        return true;
    }

    // initialize queue.
    queue_init(&q);

    // initialize mmap list.
    m->head = NULL;
    m->tail = NULL;

    return false;
}


bool analyze()
{
    char buf[512];
    mm_node *mnode;

    // read line from read and candidate logs.
    if (!fgets(buf, sizeof(buf), fp_read)) {
        // end of analysis.
        return false;
    }

    if ((buf[0] == 'm') && (buf[2] == 'e')) {
        mnode = new_mmap_node();
        mnode->md = gen_message_digest(mnode->path);

        insert_mm_list(mm_list, mnode);
    }
}