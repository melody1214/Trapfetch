#include "tracer.h"

long long get_timestamp()
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (long long) (ts.tv_sec * pow(10, 9) + ts.tv_nsec);
}

static void write_mmap_log(struct user_regs_struct *regs, char *fname, long long timestamp)
{
	int md;
	int prot;
	char prot_to_char;
	long long off, len;
	void *mm_start, *mm_end;

	md = hash(fname);
	off = regs->ARGS_5;
	len = regs->ARGS_1;
	prot = (int)regs->ARGS_2;

	mm_start = (void *)regs->RET;
	mm_end = (void *)(mm_start + len);

	prot_to_char = 'r';

	if (prot & 0x4) {
		prot_to_char = 'e';
	}
	
	fprintf(fp_read, "m,%c,%s,%lld,%lld,%lld,%d,%p,%p\n", prot_to_char, fname, timestamp, off, len, md, mm_start, mm_end);
}

int get_filepath(pid_t pid, int fd, char *filename) {
	char *buf = (char *)malloc(512 * sizeof(char));
	sprintf(buf, "/proc/%d/fd/%d", pid, fd);
	memset(filename, '\0', 512 * sizeof(char));
	
	if (readlink(buf, filename, 512 * sizeof(char)) < 0) {
		//perror("readlink");
		return -1;
	}
	return 0;
}

// create_logfile generates a log file and returns a pointer of the file.
static FILE *create_logfile(char *target_name, char log_type)
{
	FILE *fp;
	char fname[512];

	memset(fname, '\0', 512 * sizeof(char));

	if (log_type == 'R') {
		strcpy(fname, LOG_PATH"/r.");
	}
	else if (log_type == 'C') {
		strcpy(fname, LOG_PATH"/c.");
	}
	else {
		return NULL;
	}

	strcat(fname, basename(target_name));

	fp = fopen(fname, OPEN_FLAG);
	if (fp == NULL) {
		perror("fopen");
		return NULL;
	}

	if (chmod(fname, OPEN_PERM) < 0) {
		perror("chmod");
		return NULL;
	}

	return fp;
}

pid_t lookup_pid(pid_t pid)
{
	int i;

	for (i = 0; i < MAX_PIDTABSIZE; i++) {
		if (pidtab[i] == pid)
			return pid;
	}

	return -1;
}

pid_t alloc_new_pid(pid_t pid)
{
	int i;

	for (i = 0; i < MAX_PIDTABSIZE; i++) {
		if (pidtab[i] == 0) {
			pidtab[i] = pid;
			nprocs++;
			return pid;
		}
	}

	return -1;
}

pid_t drop_pid(pid_t pid)
{
	int i;

	for (i = 0; i < MAX_PIDTABSIZE; i++) {
		if (pidtab[i] == pid) {
			pidtab[i] = 0;
			nprocs--;
			return pid;
		}
	}

	return -1;
}

static int ptrace_restart(const unsigned int op, pid_t pid, unsigned int sig)
{
	int err;

	errno = 0;
	ptrace(op, pid, 0L, (unsigned long) sig);
	err = errno;

	if (!err)
		return 0;

	if (err == ESRCH)
		return 0;

	errno = err;
	perror("ptrace_restart");
	return -1;
}

static int ptrace_seize(pid_t pid, unsigned int options)
{
	if (ptrace(PTRACE_SEIZE, pid, 0L, (unsigned int)options) < 0) {
		perror("ptrace_seize");
		return -1;
	}

	if (ptrace(PTRACE_INTERRUPT, pid, 0L, 0L) < 0) {
		perror("ptrace_interrupt");
		return -1;
	}

	return 0;
}

static int ptrace_getinfo(const unsigned int op, pid_t pid, void *info) {
	int err;

	errno = 0;
	ptrace(op, pid, 0, &info);
	err = errno;

	if (!err)
		return 0;

	perror("ptrace_getinfo")
	return -1;
}


bool trace(void)
{
	pid_t tracee;

	int wait_errno;
	int wait_status;
	int md;
	long long timestamp;
	bool stopped;

	unsigned int sig, event;
	unsigned long eventmsg, off, len;
	unsigned long long inst;

	void *ret_addr;
	
	struct stat fstatus;
	//struct user_regs_struct regs;

	char buf[512], fname[512];

	tracee = waitpid(-1, &wait_status, __WALL);

	if (tracee < 0) {
		wait_errno = errno;
		
		if (wait_errno == EINTR)
			return true;
		if (nprocs == 0 && wait_errno == ECHILD) {
			return false;
		}

		printf("nprocs : %d, errno : %d\n", nprocs, wait_errno);
		exit(EXIT_FAILURE);
	}

	if (lookup_pid(tracee) < 0) {
		if (alloc_new_pid(tracee) < 0) {
			perror("alloc_new_pid : pidtab is full");
			exit(EXIT_FAILURE);
		}
	}

	if (WIFSTOPPED(wait_status))
		ptrace(PTRACE_GETREGS, tracee, 0, &regs);

	event = wait_status >> 16;

	if (event == PTRACE_EVENT_EXEC) {
		if (ptrace(PTRACE_GETEVENTMSG, tracee, NULL, &eventmsg) < 0) {
			perror("ptrace_geteventmsg");
			exit(EXIT_FAILURE);
		}

		if ((unsigned long)eventmsg != (unsigned long)tracee) {
			if (drop_pid((pid_t)eventmsg) < 0) {
				perror("drop_pid");
				exit(EXIT_FAILURE);
			}
		}
	}

	if (WIFSIGNALED(wait_status)) {
		if (drop_pid(tracee) < 0) {
			perror("drop_pid on WIFSIGNALED\n");
			exit(EXIT_FAILURE);
		}
		return true;
	}

	if (WIFEXITED(wait_status)) {
		if (drop_pid(tracee) < 0) {
			perror("drop_pid on WIFEXITED\n");
			exit(EXIT_FAILURE);
		}
		return true;
	}

	if (!WIFSTOPPED(wait_status)) {
		if (drop_pid(tracee) < 0) {
			perror("drop_pid on !WIFSTOPPED\n");
			exit(EXIT_FAILURE);
		}
		return true;
	}

	// get a signal number.
	sig = WSTOPSIG(wait_status);

	switch (event) {
		case 0:
			if ((sig == SIGTRAP) || (sig == SYSCALL_STOP)) {
				ptrace_getinfo(PTRACE_GETREGS, tracee, &regs);

#if ARCH == 32
				if (regs.ORIG_AX != SYS_mmap2) {
					if (regs.ORIG_AX != SYS_mmap) {
						goto restart;
					}
				}
#else
				if (regs.ORIG_AX != SYS_mmap) {
					goto restart;
				}
#endif
				// tracing only if fd value for mmap is greater than 3
				if ((int)regs.ARGS_4 < 3) {
					goto restart;
				}

				// syscall-entry stop
				if (insyscall == 0) {
					insyscall = 1;
					timestamp = get_timestamp();
					goto restart;
				}

				// syscall-exit stop
				insyscall = 0;
				if (get_filepath(tracee, (int)regs.ARGS_4, fname) < 0) {
					goto restart;
				}
					
				stat(fname, &fstatus);
				if (!S_ISREG(fstatus.st_mode)) {
					goto restart;
				}

				// write log.	
				write_mmap_log(&regs, fname, timestamp);
			}
			else {
				stopped = ptrace(PTRACE_GETSIGINFO, tracee, 0, &si) < 0;
				
				if (!stopped) {
					if (ptrace_restart(PTRACE_SYSCALL, tracee, sig) < 0)
						exit(EXIT_FAILURE);
				}
				else {
					if (ptrace_restart(PTRACE_LISTEN, tracee, 0) < 0)
						exit(EXIT_FAILURE);
				}
			}
			return true;
		case PTRACE_EVENT_EXIT:
			if (tracee == thread_leader) {
				printf("leader exit with nprocs : %d\n", nprocs);
			}
		case PTRACE_EVENT_STOP:
			switch (sig) {
				case SIGSTOP:
				case SIGTSTP:
				case SIGTTIN:
				case SIGTTOU:
					if (ptrace_restart(PTRACE_LISTEN, tracee, 0) < 0)
						exit(EXIT_FAILURE);
					return true;
			}
	}

restart:
	if (ptrace_restart(PTRACE_SYSCALL, tracee, 0) < 0) {
		exit(EXIT_FAILURE);
	}
	return true;
}

/* set up tracing a target application */
void startup_child(int argc, char **argv)
{
	pid_t tracee;

	unsigned int wait_status;
	char fname[512];

	struct stat fstatus;
	long long timestamp;

	if ((tracee = fork()) < 0) {
		perror("fork");
		exit(EXIT_FAILURE);
	}
	
	// tracee
	if (tracee == 0) {
		raise(SIGSTOP);

		char *dirc = strndup(argv[1], strlen(argv[1]));
		char *basec = strndup(argv[1], strlen(argv[1]));
		char *dname = dirname(dirc);
		char *bname = basename(basec);

		// set library path to target application's directory.
		setenv("LD_LIBRARY_PATH", dname, 1);
		
		// TARGET_PROGRAM is used in writing a log by the tracer and wrapper.
		setenv("TARGET_PROGRAM", bname, 1);
		chdir(dname);

		free(dirc);
		free(basec);

#if ARCH == 64
		setenv("LD_PRELOAD", "/home/melody/study/projects/trapfetch/bin/wrapper.so", 1);
#else
		setenv("LD_PRELOAD", "/home/melody/study/projects/trapfetch/bin/wrapper_32.so", 1);
#endif

		printf("getenv(LD_PRELOAD) : %s\n", getenv("LD_PRELOAD"));
		printf("getenv(TARGET_PROGRAM) : %s\n", getenv("TARGET_PROGRAM"));

		if (execv(argv[1], &argv[1]) < 0) {
			perror("execv");
			exit(EXIT_FAILURE);
		}
	}

	// tracer
	if ((tracee = waitpid(-1, &wait_status, WSTOPPED)) < 0) {
		perror("waitpid");
		exit(EXIT_FAILURE);
	}

	// Create log files.
	// 'R' means read, and 'C' means candidate.	
	fp_read = create_logfile(argv[1], 'R');
	fp_candidates = create_logfile(argv[1], 'C');
	if ((fp_read == NULL) || (fp_candidates == NULL)) {
		perror("Failed to log file creation")
		exit(EXIT_FAILURE);
	}

	thread_leader = tracee;

	memset(pidtab, 0, 256 * sizeof(pid_t));
	pidtab[0] = tracee;
	nprocs++;

	insyscall = 0;

	// attach ptrace() to tracee, and stop it.
	if (ptrace_seize(tracee, PT_OPTIONS) < 0)
		exit(EXIT_FAILURE);

	// examine tracee's current registers.
	ptrace_getinfo(PTRACE_GETREGS, tracee, &regs);

	/* 
	memset(fname, '\0', 512 * sizeof(char));
	realpath(argv[1], fname);
	if (stat(fname, &fstatus) < 0) {
		perror("stat");
		exit(EXIT_FAILURE);
	}

#if ARCH ==	32
	regs.RET = 0x2000000;
	regs.ARGS_5 = 0;
	regs.ARGS_1	= fstatus.st_size;
#else
	regs.RET = 0x400000;
	regs.ARGS_5 = 0;
	regs.ARGS_1 = fstatus.st_size;
#endif

	timestamp = get_timestamp();

	if (S_ISREG(fstatus.st_mode))
		write_mmap_log(&regs, fname, timestamp);

	*/

	kill(tracee, SIGCONT);
}