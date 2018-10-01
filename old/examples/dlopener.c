#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>


void print_hello(void)
{
	open("/home/shared/prefetch/logs/mm_fgfs", O_RDONLY);
	printf("\nhello_print\n");
	
}

int main(int argc, char const *argv[]){
	pid_t child;
	int wait_status;
	void *dlhandle_1, *dlhandle_2;
	char buf[128];
	int fd;

	child = fork();

	if (child == 0) {
		dlhandle_2 = dlopen("/home/shared/prefetch/dlreader_2.so", RTLD_NOW);
		if (!dlhandle_2) {
			fprintf(stderr, "%s\n", dlerror());
			exit(EXIT_FAILURE);
		}
		fd = open("/home/shared/prefetch/examples/dlopener.c", O_RDONLY);
		if (fd < 0) {
			perror("open");
			return -1;
		}


		void (*dlfunc_2)(void) = dlsym(dlhandle_2,"readtext");
		dlfunc_2();
		
		while (read(fd, buf, 128));
		
		dlclose(dlhandle_2);
		close(fd);	
	}
	else {
		wait(&wait_status);
		
		print_hello();
		dlhandle_1 = dlopen("/home/shared/prefetch/dlreader_1.so", RTLD_NOW);
		if (!dlhandle_1){
			fprintf(stderr, "%s\n", dlerror());
			exit(EXIT_FAILURE);
		}
		fd = open("/home/shared/prefetch/examples/helloworld", O_RDONLY);
		if (fd < 0) {
			perror("open");
			return -1;
		}

		void (*dlfunc_1)(void) = dlsym(dlhandle_1,"readtext");
		dlfunc_1();
		
		while (read(fd, buf, 128));

		dlclose(dlhandle_1);
		close(fd);
	}
	return 0;
}
