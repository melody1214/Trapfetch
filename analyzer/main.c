#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	// int fd_read, fd_candidates, fd_bp, fd_pf;

	if (an_init() < 0) {
		fprintf(stderr, "failed to init analyzer\n");
		return -1;
	}

	
	/*
	queue *q;
	mmap_list *m_list;
	mmap_node *m_node;
	read_node *r_node;
	cand_node *c_node;
	*/

	// fd_read = get_fd(argv[1], LOGPATH"/read_", OPEN_READ);
	// fd_candidates = get_fd(argv[1], LOGPATH"/cand_", OPEN_READ);
	// fd_bp = get_fd(argv[1], LOGPATH"/bp_", OPEN_WRITE);
	// fd_pf = get_fd(argv[1], LOGPATH"/pf_", OPEN_WRITE);

	//q = init_queue();
	
	return 0;
}

