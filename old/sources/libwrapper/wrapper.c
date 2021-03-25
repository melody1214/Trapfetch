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

#include <semaphore.h>
#include <sys/time.h>

#define	LBN_START	929523712
#define	SECTOR_SIZE	512
#define	EXTENT_MAX_COUNT	512

static FILE *fp_log = NULL;
static FILE *fp_candidates = NULL;

long long gettime()
{
	struct timespec ts;
	
	//CLOCK_MONOTONIC : monotonic time since some unspecified starting point. This clock is not affected by discontinuous jumps in the system time.
	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (long long) (ts.tv_sec * pow(10, 9) + ts.tv_nsec);
}

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
		
	}
	*/

	fe_physical_start = fiemap->fm_extents[0].fe_physical;
	free(fiemap);
	return (fe_physical_start / SECTOR_SIZE) + LBN_START;
}

ssize_t read(int fildes, void *buf, size_t nbyte)
{
	ssize_t (*original_read)(int fildes, void *buf, size_t nbyte);
	loff_t pos;
	pid_t pid;
	long lba_start;
	long long timestamp;
	ssize_t ret;
	char path[512];
	char fname[512];

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

	// get the first LBN of a file.
	lba_start = fiemap(fildes, (unsigned long long)pos);
	
	// fiemap() failed. (not regular file)
	if (lba_start < 0)
		return ret;

	if (fp_log == NULL)
		fp_log = fopen("/home/melody/work/trapfetch/logs/log", "a");


	// get timestamp.
	timestamp = gettime();

	// record the log. {path, timestamp, file offset, length and LBN}
	fprintf(fp_log, "%s,%lld,%d,%d,%ld\n", fname, timestamp, (int)pos, (int)ret, lba_start);

	// return the return value of the original read.
	return ret;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	size_t (*original_fread)(void *ptr, size_t size, size_t nmemb, FILE *stream);
	size_t ret;
	loff_t pos;
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

	pos = ftell(stream);

	original_fread = dlsym(RTLD_NEXT, "fread");
	ret = (*original_fread)(ptr, size, nmemb, stream);

	if (ret <= 0)
		return ret;

	lba_start = fiemap(fildes, (unsigned long long)pos);

	if (lba_start < 0)
		return ret;

	if (fp_log == NULL)
		fp_log = fopen("/home/melody/work/trapfetch/logs/log", "a");

	timestamp = gettime();

	fprintf(fp_log, "%s,%lld,%d,%d,%ld\n", fname, timestamp, (int)pos, (int)(ret * size), lba_start);

	return ret;
}

void *memset(void *s, int c, size_t n)
{
	void *(*original_memset)(void *s, int c, size_t n);
	void *ret;
	long long timestamp;

	original_memset = dlsym(RTLD_NEXT, "memset");
	ret = original_memset(s, c, n);

	if (fp_candidates == NULL)
		fp_candidates = fopen("/home/melody/work/trapfetch/logs/log_candidates", "a");
	
	timestamp = gettime();

	fprintf(fp_candidates, "%p,%lld\n", __builtin_return_address(0), timestamp);

	return ret;
}

/*
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	int (*original_gettimeofday)(struct timeval *tv, struct timezone *tz);
	int ret;
	long long timestamp;

	original_gettimeofday = dlsym(RTLD_NEXT, "gettimeofday");
	ret = original_gettimeofday(tv, tz);

	if (fp_candidates == NULL)
		fp_candidates = fopen("/home/melody/work/trapfetch/logs/log_candidates", "a");

	timestamp = gettime();

	fprintf(fp_candidates, "%p,%lld\n", __builtin_return_address(0), timestamp);

	return ret;
}
*/

/*
void *memmove(void *dest, const void *src, size_t n)
{
	void *(*original_memmove)(void *, const void *, size_t);
	void *ret;
	long long timestamp;

	original_memmove = dlsym(RTLD_NEXT, "memmove");
	ret = original_memmove(dest, src, n);

	if (fp_candidates == NULL)
		fp_candidates = fopen("/home/melody/work/trapfetch/logs/log_candidates", "a");

	timestamp = gettime();

	fprintf(fp_candidates, "%p,%lld\n", __builtin_return_address(0), timestamp);

	return ret;
}
*/

/*
int sem_wait(sem_t *sem)
{
	int (*original_sem_wait)(sem_t *sem);
	int ret;
	long long timestamp;

	original_sem_wait = dlsym(RTLD_NEXT, "sem_wait");
	ret = original_sem_wait(sem);

	if (fp_candidates == NULL)
		fp_candidates = fopen("/home/melody/work/trapfetch/logs/log_candidates", "a");

	timestamp = gettime();

	fprintf(fp_candidates, "%p,%lld\n", __builtin_return_address(0), timestamp);

	return ret;
}
*/

char *strcpy(char *dest, const char *src)
{
	char *(*original_strcpy)(char *dest, const char *src);
	char *ret;
	long long timestamp;

	original_strcpy = dlsym(RTLD_NEXT, "strcpy");
	ret = original_strcpy(dest, src);
	
	if (fp_candidates == NULL)
		fp_candidates = fopen("/home/melody/work/trapfetch/logs/log_candidates", "a");

	timestamp = gettime();

	fprintf(fp_candidates, "%p,%lld\n", __builtin_return_address(0), timestamp);

	return ret;
}


/*
char *strncpy(char *dest, const char *src, size_t n)
{
	char *(*original_strncpy)(char *dest, const char *src, size_t n);
	char *ret;
	long long timestamp;

	original_strncpy = dlsym(RTLD_NEXT, "strncpy");
	ret = original_strncpy(dest, src, n);
	
	if (fp_candidates == NULL)
		fp_candidates = fopen("/home/melody/work/trapfetch/logs/log_candidates", "a");

	timestamp = gettime();

	fprintf(fp_candidates, "%p,%lld\n", __builtin_return_address(0), timestamp);

	return ret;
}
*/

void *memcpy(void *dest, const void *src, size_t n)
{
	void *(*original_memcpy)(void *dest, const void *src, size_t n);
	void *ret;
	long long timestamp;

	original_memcpy = dlsym(RTLD_NEXT, "memcpy");
	ret = original_memcpy(dest, src, n);

	if (fp_candidates == NULL)
		fp_candidates = fopen("/home/melody/work/trapfetch/logs/log_candidates", "a");

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
		fp_candidates = fopen("/home/melody/work/trapfetch/logs/log_candidates", "a");

	timestamp = gettime();

	fprintf(fp_candidates, "%p,%lld\n", __builtin_return_address(0), timestamp);

	return ret;
}

/*
int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	int (*original_pthread_mutex_lock)(pthread_mutex_t *mutex);
	int ret;
	long long timestamp;

	original_pthread_mutex_lock = dlsym(RTLD_NEXT, "pthread_mutex_lock");
	ret = (*original_pthread_mutex_lock)(mutex);

	if (fp_candidates == NULL)
		fp_candidates = fopen("/home/melody/work/trapfetch/logs/log_candidates", "a");

	timestamp = gettime();

	fprintf(fp_candidates, "%p,%lld\n", __builtin_return_address(0), timestamp);

	return ret;
}
*/

/*
int pthread_mutext_unlock(pthread_mutex_t *mutex)
{
	int (*original_pthread_mutex_unlock)(pthread_mutex_t *mutex);
	int ret;
	long long timestamp;

	original_pthread_mutex_unlock = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
	ret = (*original_pthread_mutex_unlock)(mutex);

	if (fp_candidates == NULL)
		fp_candidates = fopen("/home/melody/work/trapfetch/logs/log_candidates", "a");

	timestamp = gettime();

	fprintf(fp_candidates, "%p,%lld\n", __builtin_return_address(0), timestamp);

	return ret;
}
*/
