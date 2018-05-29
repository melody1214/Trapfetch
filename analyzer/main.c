#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

#define	LOGPATH	"/home/melody/study/projects/trapfetch/logs"
#define	OPEN_READ 	0
#define	OPEN_WRITE 	1

int get_fd(char *path, char *dir, int flag)
{
	char fname[512];
	int fd;

	memset(fname, '\0', 512 * sizeof(char));
	strcpy(fname, dir);
	strcat(fname, path);

	if (flag == OPEN_READ) {
		if ((fd = open(fname, O_RDONLY)) < 0) {
			perror("open");
			exit(EXIT_FAILURE);
		}
	}
	else {
		if ((fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC)) < 0) {
			perror("open");
			exit(EXIT_FAILURE);
		}
	}

	return fd;
}

int main(int argc, char **argv)
{
	int fd_read, fd_candidates, fd_bp, fd_pf;

	/*
	queue *q;
	mmap_list *m_list;
	mmap_node *m_node;
	read_node *r_node;
	cand_node *c_node;
	*/

	fd_read = get_fd(argv[1], LOGPATH"/read_", OPEN_READ);
	fd_candidates = get_fd(argv[1], LOGPATH"/cand_", OPEN_READ);
	fd_bp = get_fd(argv[1], LOGPATH"/bp_", OPEN_WRITE);
	fd_pf = get_fd(argv[1], LOGPATH"/pf_", OPEN_WRITE);

	//q = init_queue();
	
	return 0;
}

