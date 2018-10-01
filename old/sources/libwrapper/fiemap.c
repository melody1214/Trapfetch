#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <math.h>
#include <time.h>
#include <fcntl.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/fiemap.h>
#include <linux/fs.h>

long long fiemap(int fd, long long pos)
{
	int i;
	int blocksize;
	char *fiebuf;
	struct fiemap *fiemap;
	long long fe_logical_start; // logical byte offset of extent
	long long fe_physical_start; // physical byte offset of extent
	long long fe_length; // the number of bytes in extent

	uint count = 32;
	fiebuf = malloc(sizeof (struct fiemap) + (count * sizeof(struct fiemap_extent)));

	if (!fiebuf) {
		perror("Could not allocate fiemap buffers");
		exit(1);
	}

	fiemap = (struct fiemap *)fiebuf;
	fiemap->fm_start = 0;
	fiemap->fm_length = ~0ULL;
	fiemap->fm_flags = 0;
	fiemap->fm_extent_count = count;
	fiemap->fm_mapped_extents = 0;

	if (ioctl(fd, FIGETBSZ, &blocksize) < 0) {
		perror("Can't get block size");
		return -1;
	}

	// get extent information of a file.
	if (ioctl(fd, FS_IOC_FIEMAP, (unsigned long)fiemap) < 0) {
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

	// Find the extent that contains the current file position and return the physical byte offset of the extent.
	for (i = 0; i < fiemap->fm_mapped_extents; i++) {
		fe_logical_start = fiemap->fm_extents[i].fe_logical;
		fe_physical_start = fiemap->fm_extents[i].fe_physical;
		fe_length = fiemap->fm_extents[i].fe_length;

		if ((fe_logical_start <= pos)
				&& (pos <= (fe_logical_start + fe_length)))
			return fe_physical_start;
	}

	return -1;
}

int main()
{
	int fd, fd2;

	fd = open("/home/melody/sample.c", O_RDONLY);
	fd2 = open("/home/melody/reglist_161228.csv", O_RDONLY);
	printf("sample.c : %lld\n", fiemap(fd, 0));
	printf("reglist_161228.csv : %lld\n", fiemap(fd2, 0));


	return 0;

}
