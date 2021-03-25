#include "file.h"

FILE *get_fp(char *path) {
  FILE *fp;

  if ((fp = fopen(path, "r")) == NULL) {
    perror("fopen");
    return NULL;
  }

  return fp;
}