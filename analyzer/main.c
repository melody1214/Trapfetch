#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

extern bool an_init();
extern bool analyze();
extern void reordering_pf_list();
extern void reordering_read_list();
extern void merging_read_list();

int main(int argc, char **argv) {
  if (!an_init(argv)) {
    fprintf(stderr, "failed to init analyzer\n");
    return -1;
  }

  // analyze() reads all of the logs written by tracer, and classifies each
  // period into burst or idle.
  while (analyze()) {
    ;
  }

  reordering_pf_list();

  reordering_read_list();

  merging_read_list();

  return 0;
}