#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

extern bool startup_child(int argc, char **argv);
extern bool trace(void);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: <prefetcher> <target app>\n");
    exit(EXIT_FAILURE);
  }

  if (!startup_child(argc, argv)) {
    perror("startup_child");
    return -1;
  }

  while (trace())
    ;
  
  printf("\nprefetcher: every tracees have been terminated normally\n");
  
  return 0;
}