/*
	Caller
	Make data structures
	Loop until Application terminated
		Case
			IF Meet breakpoint
				Do prefetch

			IF MMAP
				Insert breakpoint

			IF FORK
				Turn tracee to child

			IF exit
				Turn tracee to parents
	Loop end
*/
#define	_GNU_SOURCE
#define	fgfs

#include <sys/ptrace.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/user.h>		//user_regs_struct
#include <sys/syscall.h>	//SYS_mmap variable
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <libgen.h>			//for Using basename
#include <signal.h>
#include <string.h>			// memcpy()
#include <fcntl.h>
#include <pthread.h>

#include "../header/hash.h"
#include "../header/structures.h"

#if	defined(sd2) || defined(fgfs)
	#ifdef	sd2
		#include "../header/sequence_speed-dreams-2.h"
	#else
		#include "../header/sequence_fgfs.h"
	#endif
#elif	defined(eclipse) || defined(vegastrike)
	#ifdef	eclipse
		#include "../header/sequence_eclipse.h"
	#else
		#include "../header/sequence_vegastrike.h"
	#endif
#endif


#define TARGET_ADDRESS regs.rax+bp_offset
#define BUFFER_SIZE 255

/*void* do_posix_fadvise(void* data) {
	f_node* target = (f_node*)data;
	int temp_fd = open(target->filepath, O_RDONLY);
	posix_fadvise(temp_fd, target->offset, target->len, POSIX_FADV_WILLNEED);
	close(temp_fd);
	pthread_exit((void*)0);
	return NULL;
}*/

int main(int argc, char *argv[]) {
	struct user_regs_struct regs;
	unsigned long long backup_data = 0;
	unsigned long long bpcode;
	unsigned long long bp_offset;
	int status;
	int ret;
	int temp_fd;
	int bp_counter = 1;
	int insyscall = 0;
	int i = 0;
	int nr_to_read;
	char* app_name = basename(argv[1]);
	//char buf[4096];
	//offset_list* o_list = newOffsetList();

	if (argc < 2) {
		printf("Usage ./caller <target>\n");
		exit(0);
	}
	/*
	char pf_prefix[BUFFER_SIZE] = "/home/shared/prefetch/logs/pf_mmap_";

	// Load prefetch Sequence for Prefetch
	fl_list* fllist = newFLlist();
	FILE* pf_file = fopen(strcat(pf_prefix, app_name), "r");
	if (pf_file == NULL) {
		printf("ERROR, pf_fd\n");
		exit(-1);
	}
	char pf_read_buffer[BUFFER_SIZE];
	while ( fgets(pf_read_buffer, BUFFER_SIZE, pf_file)) {
		int pf_md;
		unsigned long long pf_bp_offset;
		char pf_filepath[255];
		long pf_offset;
		long pf_len;
		memset(pf_filepath, '\0', 255 * sizeof(char));
		sscanf(pf_read_buffer, "%d,%llx,%[^,],%ld,%ld\n", &pf_md, &pf_bp_offset, pf_filepath, &pf_offset, &pf_len);
		f_node* new_fnode = newFNode(pf_filepath, pf_offset, pf_len);
		f_list* temp_flist = getFList(fllist, pf_md, pf_bp_offset);
		if (temp_flist == NULL) {
			f_list* new_flist = newFList(pf_md, pf_bp_offset);
			appendFNode(new_flist, new_fnode);
			appendFList(fllist, new_flist);
		}
		else {
			appendFNode(temp_flist, new_fnode);
		}
	}*/
	/* Prefetch */
	//f_list* prefetch_flist = fllist->head;

	while ( i < NUMOFARRAY ) {
		if ((temp_fd = open(pf_array[i].path, O_RDONLY | O_NONBLOCK)) < 0) {
			printf("open failed for %s\n", pf_array[i].path);
			exit(-1);
		}

		nr_to_read = 0;

		while (((pf_array[i].length - nr_to_read) > 0)) {
			if ((pf_array[i].length - nr_to_read) >= 131072) {
				if ((ret = posix_fadvise(temp_fd, nr_to_read, 131072, POSIX_FADV_WILLNEED)) < 0) {
					printf("posix_fadvise failed for %s\n", pf_array[i].path);
					exit(-1);
				}
				nr_to_read += 131072;
			}
			else {
				if ((ret = posix_fadvise(temp_fd, nr_to_read, pf_array[i].length - nr_to_read, POSIX_FADV_WILLNEED)) < 0) {
					printf("posix_fadvise failed for %s\n", pf_array[i].path);
					exit(-1);
				}
				nr_to_read += pf_array[i].length - nr_to_read;
			}
		}
		close(temp_fd);
		/*	
		close(temp_fd);

		if ((temp_fd = open(pf_array[50].path, O_RDONLY | O_NONBLOCK)) < 0) {
			printf("open failed for %s\n", pf_array[i].path);
			exit(-1);
		}
	
		if ((ret = posix_fadvise(temp_fd, pf_array[50].offset, pf_array[50].length, POSIX_FADV_WILLNEED)) < 0) {
			printf("posix_fadvise failed for %s\n", pf_array[50].path);
			exit(-1);
		}
			
		close(temp_fd);
		*/

		i++;
	}
		/*while ((ret = read(temp_fd, buf, sizeof(buf)))) {
			if (ret < 0) {
				printf("posix_fadvise failed for %s\n", pf_array[i].path);
				exit(-1);
			}
		}*/

	int child = fork();
	if (child == 0 ) {
		execl(argv[1], argv[1], NULL);
	}
	else {
		wait(&status);
		printf("prefetcher's pid : %d, application's pid : %d\n", getpid(), child);
	}
	return 0;
}

