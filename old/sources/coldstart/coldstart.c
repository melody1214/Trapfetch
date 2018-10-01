//#define	_GNU_SOURCE
#include <stdio.h>
#include <wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>

int main(int argc, char **argv)
{
	int pid;
	int wait_status;

	/*
	cpu_set_t set;
	CPU_ZERO(&set);
	CPU_SET(0, &set);
	sched_setaffinity(0, sizeof(cpu_set_t), &set);
*/

	if (argc != 2) {
		printf("Usage: ./coldstart <Target app's path>\n");
		exit(EXIT_FAILURE);
	}

	if ((pid = fork()) < 0) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if (pid == 0) {
		if (execl(argv[1], argv[1], NULL) < 0) {
			perror("execl");
			exit(EXIT_FAILURE);
		}
	}
	
	pid = waitpid(-1, &wait_status, __WALL);
	return 0;
	
}
