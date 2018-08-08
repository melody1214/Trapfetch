#include <fcntl.h>
#include <linux/fiemap.h>
#include <linux/fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#define LBN_START 929523712
#define SECTOR_SIZE 512
#define EXTENT_MAX_COUNT 512

#define OPEN_READ 0
#define OPEN_WRITE 1

#define PATH_READ_LOG "/home/melody/study/projects/trapfetch/logs/r."
#define PATH_CANDIDATE_LOG "/home/melody/study/projects/trapfetch/logs/c."

#define PATH_PF "/home/shared/prefetch/log/pf_"
#define PATH_BP "/home/shared/prefetch/log/bp_"

FILE *get_fd(char *path, char *dir, int flag);
unsigned int get_logical_blk_addr(char *path);
