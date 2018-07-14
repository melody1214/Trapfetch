#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	// int fd_read, fd_candidates, fd_bp, fd_pf;

	if (an_init()) {
		fprintf(stderr, "failed to init analyzer\n");
		return -1;
	}

	while (analyze()) {
		
	}
	
	return 0;
}

