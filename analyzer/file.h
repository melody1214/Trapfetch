#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/fiemap.h>
#include <linux/fs.h>

#define LBN_START 929523712
#define SECTOR_SIZE 512
#define EXTENT_MAX_COUNT 512

#define OPEN_READ 0
#define OPEN_WRITE 1

#define PATH_READ_LOG "/home/melody/study/projects/trapfetch/logs/r."
#define PATH_CANDIDATE_LOG "/home/melody/study/projects/trapfetch/logs/c."

FILE *get_fd(char *path, char *dir, int flag);
unsigned int get_logical_blk_addr(char *path);
