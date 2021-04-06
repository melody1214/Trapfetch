#define _GNU_SOURCE
#define	FILE_OFFSET_BITS	64
#define	LARGEFILE_SOURCE

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <math.h>
#include <time.h>

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/fiemap.h>
#include <linux/fs.h>

#define	LBN_START	929523712
#define	SECTOR_SIZE	512
#define	EXTENT_MAX_COUNT	512

/*
#define READ_PATH	"/dev/shm/r."
#define CANDIDATE_PATH	"/dev/shm/c."
*/

#define READ_PATH	"/home/melody/work/trapfetch/logs/r."
#define CANDIDATE_PATH	"/home/melody/work/trapfetch/logs/c."


#define	OPEN_FLAG	"a"

static FILE *fp_log = NULL;
static FILE *fp_fcandidates = NULL;

long long gettime()
{
	struct timespec ts;
	
	//CLOCK_MONOTONIC : monotonic time since some unspecified starting point. This clock is not affected by discontinuous jumps in the system time.
	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (long long) (ts.tv_sec * pow(10, 9) + ts.tv_nsec);
}

/*
__u64 fiemap(int fd, __u64 pos)
{
	int i;
	int blocksize;
	struct fiemap *fiemap;
	__u64 fe_logical_start; // logical byte offset of extent
	__u64 fe_physical_start; // physical byte offset of extent
	__u64 fe_length; // the number of bytes in extent

	fiemap = malloc(sizeof (struct fiemap) + (EXTENT_MAX_COUNT * sizeof(struct fiemap_extent)));

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

	// get extent information of a file.
	if (ioctl(fd, FS_IOC_FIEMAP, fiemap) < 0) {
		return -1;
	}

	// Find the extent that contains the current file position and return the physical byte offset of the extent.
	for (i = 0; i < fiemap->fm_mapped_extents; i++) {
		fe_logical_start = fiemap->fm_extents[i].fe_logical;
		fe_physical_start = fiemap->fm_extents[i].fe_physical;
		fe_length = fiemap->fm_extents[i].fe_length;

		if ((fe_logical_start <= pos)
				&& (pos <= (fe_logical_start + fe_length)))
			return (fe_physical_start / SECTOR_SIZE) + LBN_START;
	}

	return -1;
}
*/

ssize_t read(int fildes, void *buf, size_t nbyte)
{
	ssize_t (*original_read)(int fildes, void *buf, size_t nbyte);
	loff_t pos;
	pid_t pid;
	long long timestamp;
	ssize_t ret;
	char path[512];
	char fname[512];

	struct stat fstatus;

	// get caller's(application's) pid
	pid = getpid();
	// If running process opens files, symbolic links of files are created in /proc/<pid>/fd/.
	sprintf(path, "/proc/%d/fd/%d", pid, fildes);
	memset(fname, '\0', sizeof(fname));

	// get real path of /proc/<pid>/fd/<fd>.
	readlink(path, fname, sizeof(fname));

	// get current file offset.
	pos = lseek(fildes, 0, SEEK_CUR);

	// obtain address of a original read().
	original_read = dlsym(RTLD_NEXT, "read");
	
	// call original read and obtain the number of bytes read(return value).
	ret = (*original_read)(fildes, buf, nbyte);

	// 0 : end of file, -1 : error
	if (ret <= 0)
		return ret;

	// check whether the file is a regular file
	stat(fname, &fstatus);
	switch (fstatus.st_mode & S_IFMT) {
		case S_IFREG:
		case S_IFLNK:
			break;
		default:
			return ret;
	}

	if (fname[0] != '/') {
		return ret;
	}

	if (fname[1] == 'r' && fname[2] == 'u' && fname[3] == 'n') {
		return ret;
	}

	if (fname[1] == 's' && fname[2] == 'y' && fname[3] == 's') {
		return ret;
	}

	if (fstatus.st_size == 0) {
		return ret;
	}

	if (fp_log == NULL) {
		memset(path, '\0', sizeof(path));
		strcpy(path, READ_PATH);
		strcat(path, basename(getenv("TARGET_PROGRAM")));
		fp_log = fopen(path, OPEN_FLAG);
	}

	// get timestamp.
	timestamp = gettime();

	// record the log. {path, timestamp, file offset, length and LBN}
	fprintf(fp_log, "r,%s,%lld,%ld,%ld\n", fname, timestamp, (long)pos, (long)ret);
	fflush(fp_log);
	

	// return the return value of the original read.
	return ret;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t (*original_fread)(void *ptr, size_t size, size_t nmemb, FILE *stream);
	size_t ret;
	loff_t pos;
	pid_t pid;
	long long timestamp;
	char path[512];
	char fname[512];
	int fildes;

	struct stat fstatus;

	fildes = fileno(stream);

	pid = getpid();
	sprintf(path, "/proc/%d/fd/%d", pid, fildes);
	memset(fname, '\0', sizeof(fname));

	readlink(path, fname, sizeof(fname));

	pos = ftell(stream);

	original_fread = dlsym(RTLD_NEXT, "fread");
	ret = (*original_fread)(ptr, size, nmemb, stream);

	if (ret <= 0)
		return ret;

	stat(fname, &fstatus);
	switch (fstatus.st_mode & S_IFMT) {
		case S_IFREG:
		case S_IFLNK:
			break;
		default:
			return ret;
	}

	if (fname[0] != '/') {
		return ret;
	}

	if (fname[1] == 'r' && fname[2] == 'u' && fname[3] == 'n') {
		return ret;
	}

	if (fname[1] == 's' && fname[2] == 'y' && fname[3] == 's') {
		return ret;
	}

	if (fstatus.st_size == 0) {
		return ret;
	}

	if (fp_log == NULL) {
		memset(path, '\0', sizeof(path));
		strcpy(path, READ_PATH);
		strcat(path, basename(getenv("TARGET_PROGRAM")));
		fp_log = fopen(path, OPEN_FLAG);
	}

	timestamp = gettime();

	fprintf(fp_log, "r,%s,%lld,%ld,%ld\n", fname, timestamp, (long)pos, (long)(ret * size));
	fflush(fp_log);

	return ret;
}

void *memmove(void *dest, const void *src, size_t n)
{
	void *(*original_memmove)(void *, const void *, size_t);
	void *ret;
	char path[512];
	long long timestamp;

	original_memmove = dlsym(RTLD_NEXT, "memmove");
	ret = original_memmove(dest, src, n);

	if (fp_fcandidates == NULL) {
		memset(path, '\0', sizeof(path));
		strcpy(path, CANDIDATE_PATH);
		strcat(path, basename(getenv("TARGET_PROGRAM")));
		fp_fcandidates = fopen(path, OPEN_FLAG);
	}


	timestamp = gettime();

	fprintf(fp_fcandidates, "%p,%lld\n", __builtin_return_address(0), timestamp);
	fflush(fp_log);

	return ret;
}

char *strcpy(char *dest, const char *src)
{
	char *(*original_strcpy)(char *dest, const char *src);
	char *ret;
	char path[512];
	long long timestamp;

	original_strcpy = dlsym(RTLD_NEXT, "strcpy");
	ret = original_strcpy(dest, src);
	
	if (fp_fcandidates == NULL) {
		memset(path, '\0', sizeof(path));
		strcpy(path, CANDIDATE_PATH);
		strcat(path, basename(getenv("TARGET_PROGRAM")));
		fp_fcandidates = fopen(path, OPEN_FLAG);
	}

	timestamp = gettime();

	fprintf(fp_fcandidates, "%p,%lld\n", __builtin_return_address(0), timestamp);

	return ret;
}

void *memcpy(void *dest, const void *src, size_t n)
{
	void *(*original_memcpy)(void *dest, const void *src, size_t n);
	void *ret;
	char path[512];
	long long timestamp;

	original_memcpy = dlsym(RTLD_NEXT, "memcpy");
	ret = original_memcpy(dest, src, n);

	if (fp_fcandidates == NULL) {
		memset(path, '\0', sizeof(path));
		strcpy(path, CANDIDATE_PATH);
		strcat(path, basename(getenv("TARGET_PROGRAM")));
		fp_fcandidates = fopen(path, OPEN_FLAG);
	}

	timestamp = gettime();

	fprintf(fp_fcandidates, "%p,%lld\n", __builtin_return_address(0), timestamp);

	return ret;
}

size_t strlen(const char *s)
{
	size_t (*original_strlen)(const char *s);
	size_t ret;
	char path[512];
	long long timestamp;

	original_strlen = dlsym(RTLD_NEXT, "strlen");
	ret = (*original_strlen)(s);

	if (fp_fcandidates == NULL) {
		memset(path, '\0', sizeof(path));
		strcpy(path, CANDIDATE_PATH);
		strcat(path, basename(getenv("TARGET_PROGRAM")));
		fp_fcandidates = fopen(path, OPEN_FLAG);
	}

	timestamp = gettime();

	fprintf(fp_fcandidates, "%p,%lld\n", __builtin_return_address(0), timestamp);

	return ret;
}

