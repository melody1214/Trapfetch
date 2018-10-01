#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

int main(void)
{
	int fd;
	int count = 0;
	char buf[4096];
	long lba = 936708264;
	long i = 0;
	fd = open("/usr/lib/x86_64-linux-gnu/libflite_cmu_us_kal.so.2.0.0", O_RDONLY);

	sleep(1);

	lseek(fd, 471040, SEEK_SET);
	while (read(fd, buf, 4096)) {
		break;

		//printf("lba : %ld, byte_offset : %d\n", lba, count);
		//count = count + 512;
		//lba++;
		i++;
	}

	return 0;
}
		
