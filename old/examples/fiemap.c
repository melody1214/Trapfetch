#include "./dstructure.h"
#include <linux/fiemap.h>
#include <linux/fs.h>
#include <sys/ioctl.h>

long long fiemap(int fd, loff_t pos, char *path) {
        int blocksize;
        char *fiebuf;   /* fiemap buffer / ioctl argument */
        struct fiemap *fiemap;
        int i;

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

        if (ioctl(fd, FS_IOC_FIEMAP, (unsigned long)fiemap) < 0) {
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
                return -1;
        }
        else {
                for (i = 0; i < fiemap->fm_mapped_extents; i++) {
                        if ((fiemap->fm_extents[i].fe_logical <= (int)pos / blocksize) && ((fiemap->fm_extents[i].fe_logical + fiemap->fm_extents[i].fe_length) > (int)pos / blocksize)) {
                                //return (fiemap->fm_extents[i].fe_physical / blocksize) + ((int)pos / blocksize);
                                return (fiemap->fm_extents[i].fe_physical / blocksize);
                        }
                }
        }
        return -1;
}


int main(int argc, char *argv[])
{
	pid_t traced_process, rpid;		// Current tracee, root tracee
	struct user_regs_struct regs;	// General purpose registers of tracees

	int wait_status;		// A wait status of tracee
	int fd;
	int blocksize;
	char *fiebuf;
	int i;
	long long lba;
	int insyscall;
	loff_t pos;	
	FILE *fp;
	char path[512];
	char fname[512];

	
	if (argc != 2) {	// Check weather correct or not of user's command
		printf("Usage: ./gatherer <Target app's path>\n");
		return -1;
	}
		
	if ((traced_process = fork()) == 0) {	// Child process
		if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
			perror("PTRACE_TRACEME");
			exit(-1);
		}
		raise(SIGSTOP);
		if (execl(argv[1], argv[1], NULL) == -1) {
			perror("execl");
			exit(-1);
		}
	}
	else {	// Parent process (tracer)
		wait(&wait_status);
		fp = fopen("./readsequence", "w+");

		pid_stack *pstack = newPidStack();	// Create and initiate stack for managing all tracees
		push(pstack, traced_process);
		rpid = traced_process;

		ptrace(PTRACE_SETOPTIONS, traced_process, NULL, PTRACE_O_TRACEFORK | PTRACE_O_TRACECLONE| PTRACE_O_TRACEEXIT
			| PTRACE_O_TRACEEXEC | PTRACE_O_TRACESYSGOOD | PTRACE_O_EXITKILL);
		ptrace(PTRACE_GETREGS, traced_process, NULL, &regs);
		
		ptrace(PTRACE_SYSCALL, traced_process, 0, 0); // resume execution of tracee until next system call
		
		insyscall = 0;
		do {	// start main loop for tracing the target application (tracee)
			traced_process = waitpid(-1, &wait_status, __WALL);

			if (WIFSTOPPED(wait_status)) {
				ptrace(PTRACE_GETREGS, traced_process, 0, &regs);
				if (regs.orig_rax == 0) {
					if (insyscall == 0) {
						insyscall = 1;
						
						sprintf(path, "/proc/%d/fd/%lld", traced_process, regs.rdi);
						memset(fname, '\0', sizeof(fname));

						readlink(path, fname, sizeof(fname));
						
						fd = open(fname, O_RDONLY);
						pos = lseek(fd, 0, SEEK_CUR);						

						lba = fiemap(fd, pos, fname);
						
						fprintf(fp, "%lld\t%s\n", lba, fname);							
					}
					else {
						insyscall = 0;
					}
				}
			}
			else if (WIFEXITED(wait_status)) {	// when each tracees exited except the first tracee
				if (traced_process == rpid) {
					break;
				}
				else if (pstack->top->pid == traced_process) {
					ptrace(PTRACE_DETACH, traced_process, 0, 0);
					traced_process = pop(pstack);
					printf("child process %d will be terminated\n", traced_process);
				}
			}

			ptrace(PTRACE_SYSCALL, traced_process, NULL, NULL); // resume execution untill next syscall-stop
		} while (1);
		
		printf("\n******** Every tracees were terminated normally ********\n");
	}
	return 0;
}
