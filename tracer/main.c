#include <stdbool.h>
#include <stdio.h>

extern void startup_child(int argc, char **argv);
extern bool trace(void);

#ifdef MEASURE_OVERHEAD
int num_of_syscall;
#endif

int main(int argc, char *argv[]) {
#ifdef MEASURE_OVERHEAD
  num_of_syscall = 0;
#endif

  startup_child(argc, argv);

  while (trace())
    ;

  printf("\ntracer: every tracees have been terminated normally\n");
  // printf("tracer: nprocs = %d\n", nprocs);

  /*
  printf("\ntracer: start sorting...\n");

  strcpy(path, "/home/melody/study/projects/trapfetch/script/sort.sh");
  strcat(path, basename(argv[1]));
  system(path);

  printf("\ntracer: sorting finished\n");
  */

  return 0;
}
