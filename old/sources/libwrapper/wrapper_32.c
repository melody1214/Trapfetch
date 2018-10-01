#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <math.h>
#include <time.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/fiemap.h>
#include <linux/fs.h>

#define	_LARGEFILE64_SOURCE	1
#define	_FILE_OFFSET_BITS	64

#define	LBN_START	929523712
#define	SECTOR_SIZE	512
#define	EXTENT_MAX_COUNT	512

static FILE *fp_log = NULL;
static FILE *fp_candidates = NULL;

long long gettime()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (long long) (ts.tv_sec * pow(10, 9) + ts.tv_nsec);
}

long long fiemap(int fd, unsigned long long pos, char *path)
{
	int blocksize;
	int i;
	struct fiemap *fiemap;
	unsigned long long fe_logical_start;
	unsigned long long fe_physical_start;
	unsigned long long fe_length;

	fiemap = malloc(sizeof(struct fiemap) + (EXTENT_MAX_COUNT * sizeof(struct fiemap_extent)));

	if (!fiemap) {
		perror("Could not allocate fiemap buffers");
		exit(1);
	}

	fiemap->fm_length = FIEMAP_MAX_OFFSET;
	fiemap->fm_flags |= FIEMAP_FLAG_SYNC;
	fiemap->fm_extent_count = EXTENT_MAX_COUNT;

	if (ioctl(fd, FIGETBSZ, &blocksize) < 0) {
		perror("Can't get block size");
		return -1;
	}

	if (ioctl(fd, FS_IOC_FIEMAP, fiemap) < 0) {
		/*
		if (errno == EBADF)
			printf("fd is not a valid descriptor : %s\n", path);
		else if (errno == EFAULT)
			printf("argp references an inaccessible memory area : %s\n", path);
		else if (errno == EINVAL)
			printf("request or argp is not valid : %s\n", path);
		else if (errno == ENOTTY)
			printf("fd is not associated with a character special device : %s\n", path);
		else
			printf("ioctl failed with unknown error : %s\n", path);
		*/

		return -1;
	}

	/*
	for (i = 0; i < fiemap->fm_mapped_extents; i++) {
		fe_logical_start = fiemap->fm_extents[i].fe_logical;
		fe_physical_start = fiemap->fm_extents[i].fe_physical;
		fe_length = fiemap->fm_extents[i].fe_length;

		if ((fe_logical_start <= pos)
				&& (pos <= (fe_logical_start + fe_length))) {
			free(fiemap);
			return (fe_physical_start / SECTOR_SIZE) + LBN_START;
		}
	}*/

	fe_physical_start = fiemap->fm_extents[0].fe_physical;
	free(fiemap);
	return (fe_physical_start / SECTOR_SIZE) + LBN_START;
}

ssize_t read(int fildes, void *buf, size_t nbyte)
{
	ssize_t (*original_read)(int fildes, void *buf, size_t nbyte);
	long long pos;
	pid_t pid;
	long lba_start;
	long long timestamp;
	long long ret;
	char path[512];
	char fname[512];

	pid = getpid();
	sprintf(path, "/proc/%d/fd/%d", pid, fildes);
	memset(fname, '\0', sizeof(fname));

	readlink(path, fname, sizeof(fname));

	pos = (long long)lseek(fildes, 0, SEEK_CUR);

	original_read = dlsym(RTLD_NEXT, "read");
	ret = (long long)(*original_read)(fildes, buf, nbyte);

	if (ret <= 0)
		return ret;

	lba_start = fiemap(fildes, pos, fname);
	
	if (lba_start < 0)
		return ret;

	if (fp_log == NULL)
		fp_log = fopen("/home/shared/prefetch/logs/log", "a");


	timestamp = gettime();

	fprintf(fp_log, "%s,%lld,%lld,%lld,%ld\n", fname, timestamp, pos, ret, lba_start);

	return ret;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t (*original_fread)(void *ptr, size_t size, size_t nmemb, FILE *stream);
	long long ret;
	long long pos;
	pid_t pid;
	long lba_start;
	long long timestamp;
	char path[512];
	char fname[512];
	int fildes;

	fildes = fileno(stream);

	pid = getpid();
	sprintf(path, "/proc/%d/fd/%d", pid, fildes);
	memset(fname, '\0', sizeof(fname));

	readlink(path, fname, sizeof(fname));

	pos = (long long)ftell(stream);

	original_fread = dlsym(RTLD_NEXT, "fread");
	ret = (long long)(*original_fread)(ptr, size, nmemb, stream);

	if (ret <= 0)
		return ret;

	lba_start = fiemap(fildes, pos, fname);

	if (lba_start < 0)
		return ret;

	if (fp_log == NULL)
		fp_log = fopen("/home/shared/prefetch/logs/log", "a");

	timestamp = gettime();

	fprintf(fp_log, "%s,%lld,%lld,%lld,%ld\n", fname, timestamp, pos, ret * size, lba_start);

	return ret;
}

void *memmove(void *dest, const void *src, size_t n)
{
	void *(*original_memmove)(void *, const void *, size_t);
	void *ret;
	long long timestamp;

	original_memmove = dlsym(RTLD_NEXT, "memmove");
	ret = original_memmove(dest, src, n);

	if (fp_candidates == NULL)
		fp_candidates = fopen("/home/shared/prefetch/logs/log_candidates", "a");

	timestamp = gettime();

	fprintf(fp_candidates, "%p,%lld\n", __builtin_return_address(0), timestamp);

	return ret;
}

void *memcpy(void *dest, const void *src, size_t n)
{
	void *(*original_memcpy)(void *dest, const void *src, size_t n);
	void *ret;
	long long timestamp;

	original_memcpy = dlsym(RTLD_NEXT, "memcpy");
	ret = original_memcpy(dest, src, n);

	if (fp_candidates == NULL)
		fp_candidates = fopen("/home/shared/prefetch/logs/log_candidates", "a");

	timestamp = gettime();

	fprintf(fp_candidates, "%p,%lld\n", __builtin_return_address(0), timestamp);

	return ret;
}

char *strcpy(char *dest, const char *src)
{
	char *(*original_strcpy)(char *dest, const char *src);
	char *ret;
	long long timestamp;

	original_strcpy = dlsym(RTLD_NEXT, "strcpy");
	ret = original_strcpy(dest, src);
	
	if (fp_candidates == NULL)
		fp_candidates = fopen("/home/shared/prefetch/logs/log_candidates", "a");

	timestamp = gettime();

	fprintf(fp_candidates, "%p,%lld\n", __builtin_return_address(0), timestamp);

	return ret;
}

size_t strlen(const char *s)
{
	size_t (*original_strlen)(const char *s);
	size_t ret;
	long long timestamp;

	original_strlen = dlsym(RTLD_NEXT, "strlen");
	ret = (*original_strlen)(s);

	if (fp_candidates == NULL)
		fp_candidates = fopen("/home/shared/prefetch/logs/log_candidates", "a");

	timestamp = gettime();

	fprintf(fp_candidates, "%p,%lld\n", __builtin_return_address(0), timestamp);

	return ret;
}
