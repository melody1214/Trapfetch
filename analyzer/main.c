#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

extern bool an_init();
extern bool analyze();
extern void reordering_pf_list();

int main(int argc, char **argv)
{
	if (!an_init(argv)) {
		fprintf(stderr, "failed to init analyzer\n");
		return -1;
	}

	// analyze() reads all of the logs written by tracer, and classifies each period into burst or idle.
	while (analyze()) {
		;
	}

	reordering_pf_list();
	
	return 0;
}