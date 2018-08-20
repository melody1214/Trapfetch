#include "file.h"

FILE *get_fp(char *path, char *dir) {
  FILE *fp;
  char fname[512];

  memset(fname, '\0', 512 * sizeof(char));
  strcpy(fname, dir);
  strcat(fname, path);

  if ((fp = fopen(fname, "r")) == NULL) {
    perror("fopen");
    return NULL;
  }

  return fp;
}