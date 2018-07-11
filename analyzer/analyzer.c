#include "analyzer.h"

int fd_read;
int fd_candidates;

struct queue q;
struct mm_list m;

int an_init(char **argv)
{
    // open log files.
    fd_read = get_fd(argv[1], PATH_READ_LOG, OPEN_READ);
    fd_candidates = get_fd(argv[1], PATH_CANDIDATE_LOG, OPEN_READ);

    if (fd_read < 0 || fd_candidates < 0) {
        return -1;
    }

    // initialize queue.
    queue_init(&q);

    return 0;
}
