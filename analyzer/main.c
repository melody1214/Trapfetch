#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

extern bool an_init(char **argv);
extern bool analyze();
extern void reordering_pf_list();
extern bool reordering_read_list();
extern bool merging_read_list();
extern void generate_meta_list();
extern void set_trigger();
extern void generate_prefetch_data(char **argv);

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

  while (merging_read_list());

  // Sorting to logical block address (for HDD only)
  while (reordering_read_list());
  // Merging after LBA sorting (for HDD only)
  while (merging_read_list());


  set_trigger();

  // Sorting and merging again after setting triggers for HDD only
  while (reordering_read_list());
  while (merging_read_list());


  generate_meta_list();
  
  generate_prefetch_data(argv);

  return 0;
}
