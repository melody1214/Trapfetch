#include "file.h"

FILE *get_fd(char *path, char *dir, int flag) {
  char fname[512];
  FILE *fp;

  memset(fname, '\0', 512 * sizeof(char));
  strcpy(fname, dir);
  strcat(fname, path);

  if (flag == OPEN_READ) {
    if ((fp = fopen(fname, "r")) == NULL) {
      perror("fopen");
      return NULL;
    }
  } else {
    if ((fp = fopen(fname, "w+")) == NULL) {
      perror("fopen");
      return NULL;
    }
  }

  return fp;
}

unsigned int get_logical_blk_addr(char *path) {
  int fd;
  int blocksize;
  struct fiemap *fiemap;

  fd = open(path, O_RDONLY);
  if (fd < 0) {
    perror("Failed to open file");
    printf("path : %s\n", path);
    return 0;
  }

  // unsigned int fe_logical_start; // logical byte offset of extent
  unsigned int fe_physical_start;  // physical byte offset of extent
  // unsigned int fe_length; // the number of bytes in extent

  fiemap = malloc(sizeof(struct fiemap) +
                  (EXTENT_MAX_COUNT * sizeof(struct fiemap_extent)));

  if (!fiemap) {
    perror("Failed to allocate fiemap buffers");
    close(fd);
    return 0;
  }

  fiemap->fm_length = FIEMAP_MAX_OFFSET;
  fiemap->fm_flags |= FIEMAP_FLAG_SYNC;
  fiemap->fm_extent_count = EXTENT_MAX_COUNT;

  if (ioctl(fd, FIGETBSZ, &blocksize) < 0) {
    perror("Failed to get block size");
    close(fd);
    return 0;
  }

  // get extent information.
  if (ioctl(fd, FS_IOC_FIEMAP, fiemap) < 0) {
    // perror("Invalid file")
    close(fd);
    return 0;
  }

  // Find the extent that contains the current file and return lba of the file.
  fe_physical_start = fiemap->fm_extents[0].fe_physical;

  close(fd);

  return (fe_physical_start / SECTOR_SIZE) + LBN_START;
}