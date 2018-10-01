#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void readtext(){
	int fd = open("/home/shared/prefetch/examples/dlreader_1.c", O_RDONLY);
	int size = 0;
	char buffer[255] = "";
	
	if(fd < 0){
		printf("open Error\n");
	}
	while((size=read(fd, buffer, 255)) > 0)
		write(2, buffer, size);
	close(fd);
}

int main(int argc, char const *argv[]){
	readtext();
	return 0;
}