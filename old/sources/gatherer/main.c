#define _GNU_SOURCE
#include "structure.h"
#include <stdbool.h>
#include <dlfcn.h>

#ifndef	PTRACE_EVENT_STOP
#	define PTRACE_EVENT_STOP 128
#endif

#define MAX_PIDTABSIZE	256

#ifndef	SYS_mmap2
#define	SYS_mmap2	192
#endif

const unsigned int syscall_trap_sig = SIGTRAP | 0x80;
unsigned int ptrace_setoptions = PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK
									| PTRACE_O_TRACECLONE | PTRACE_O_TRACEEXIT
									| PTRACE_O_TRACEEXEC | PTRACE_O_TRACESYSGOOD 
									| PTRACE_O_MMAPTRACE;

static pid_t pidtab[256];
static pid_t thread_leader;
static unsigned int pidtabsize;
static int nprocs;

static FILE *fp_etc_seq;
static FILE *fp_mm_seq;
static FILE *fp_log;
static FILE *fp_rd_seq;
static FILE *fp_log_freq;

static mmap_list *m_list;
static mmap_node *m_node;

static int insyscall;
int is_x32;

struct user_regs_struct regs;

/* Should be only used with PTRACE_CONT, PTRACE_DETACH, PTRACE_LISTEN and PTRACE_SYSCALL.
   Returns 0 on success or if error was ESRCH.
   Otherwise prints error message and returns -1. */
static int ptrace_restart (const unsigned int op, pid_t pid, unsigned int sig)
{
	int err;
	const char *msg;

	errno = 0;
	ptrace(op, pid, 0L, (unsigned long) sig);
	err = errno;

	if (!err)
		return 0;

	switch (op) {
		case PTRACE_CONT:
			msg = "CONT";
			break;
		case PTRACE_DETACH:
			msg = "DETACH";
			break;
		case PTRACE_LISTEN:
			msg = "LISTEN";
			break;
		default:
			msg = "SYSCALL";
	}

	if (err == ESRCH)
		return 0;

	errno = err;
	perror("ptrace_restart");
	return -1;
}

/* Attach to the process using PTRACE_ATTACH and PTRACE_SEIZE.
   Returns 0 on success or -1 on error with errno. */
static int ptrace_seize (pid_t pid, unsigned int options)
{
	int err;
	const char *msg;

	errno = 0;
	if (ptrace(PTRACE_SEIZE, pid, 0L, (unsigned int) options) < 0) {
		perror("ptrace_seize");
		return -1;
	}
	
	if (ptrace(PTRACE_INTERRUPT, pid, 0L, 0L) < 0) {
		perror("ptrace_interrupt");
		return -1;
	}

	return 0;
}

static void startup_child(int argc, char **argv) {
	int pid;

	unsigned int wait_status;
	char fname[512];

	struct stat fileStatus;
	long long timestamp;
	
	Ehdr elfHdr;
	Shdr sectHdr;

	/*
	if (argc != 2) {
		printf("Usage: ./gatherer <Target app's path>\n");
		exit(EXIT_FAILURE);
	}
	*/

	close(open("/home/melody/work/trapfetch/logs/log", O_WRONLY | O_CREAT | O_TRUNC, 0666));
	close(open("/home/melody/work/trapfetch/logs/log_candidates", O_WRONLY | O_CREAT | O_TRUNC, 0666));

	if ((pid = fork()) < 0) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if (pid == 0) {
		raise(SIGSTOP);

		char *dirc = strndup(argv[1], strlen(argv[1]));
		char *basec = strndup(argv[1], strlen(argv[1]));
		char *dname = dirname(dirc);
		char *bname = basename(basec);

		setenv("LD_LIBRARY_PATH", dname, 1);
		setenv("TARGET_PROGRAM", bname, 1);
		chdir(dname);

		free(dirc);
		free(basec);

		setenv("LD_PRELOAD","/home/melody/work/trapfetch/old/bin/libwrapper.so", 1);
		if (execv(argv[1], &argv[1])  < 0) {
				perror("execv");
				exit(EXIT_FAILURE);
		}
		printf("child bye\n");
	}
	
	if ((pid = waitpid(-1, &wait_status, WSTOPPED)) < 0) {
		perror("waitpid");
		exit(EXIT_FAILURE);
	}

	timestamp = get_timestamp();
	thread_leader = pid;
	insyscall = 0;

	fp_mm_seq = createSeqFiles(argv[1], fp_mm_seq, "/home/melody/work/trapfetch/logs/mm_");
	fp_rd_seq = createSeqFiles(argv[1], fp_rd_seq, "/home/melody/work/trapfetch/logs/read_");
	fp_etc_seq = createSeqFiles(argv[1], fp_etc_seq, "/home/melody/work/trapfetch/logs/etc_");

	if ((fp_mm_seq == NULL) || (fp_rd_seq == NULL) || (fp_etc_seq == NULL)) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	m_list = newMList();

	memset(fname, '\0', 512 * sizeof(char));
	realpath(argv[1], fname);
	
	memset(pidtab, 0, 256 * sizeof(pid_t));
	pidtab[0] = pid;
	nprocs++;

	if (ptrace_seize(pid, ptrace_setoptions) < 0)
		exit(EXIT_FAILURE);
	
	is_x32 = getTextsecHdr(&elfHdr, &sectHdr, fname);
	stat(fname, &fileStatus);

	if (is_x32 == 0) {
		ptrace(PTRACE_GETREGS, pid, NULL, &regs);
		regs.rax = 0x400000;
		regs.r9 = 0;
		regs.rsi = fileStatus.st_size;
		create_mmap_seq(&regs, fname, m_list, m_node, timestamp);
	}
	else {
		printf("Invalid executable : %s\n", fname);
		exit(EXIT_FAILURE);
	}
	
	kill(pid, SIGCONT);		
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

/* Returns true iff the main trace loop has to continue. */
static bool trace()
{
	pid_t traced_process;
	int wait_errno;
	int wait_status;
	int md;
	long logical_block_addr;
	long long timestamp;
	bool stopped;
	
	unsigned int sig;
	unsigned int event;
	unsigned long eventmsg;
	unsigned long ret_val_read;
	unsigned long file_offset;
	unsigned long long ret_addr;
	unsigned long long inst;

	struct stat file_status;

	char buf[512];
	char fname[512];

	traced_process = waitpid(-1, &wait_status, __WALL);
	wait_errno = errno;

	if (traced_process < 0) {
		if (wait_errno == EINTR)
			return true;
		if (nprocs == 0 && wait_errno == ECHILD) {	
			return false;
		}

		errno = wait_errno;
		perror("waitpid");
		printf("nprocs : %d, errno : %d\n", nprocs, wait_errno);
		exit(EXIT_FAILURE);
	}

	if ((int)lookup_pid(traced_process) < 0) {
		if ((int)alloc_new_pid(traced_process) < 0) {
			perror("alloc_new_pid : pidtab is full");
			exit(EXIT_FAILURE);
		}
	}
	
	if (WIFSTOPPED(wait_status))
		ptrace(PTRACE_GETREGS, traced_process, 0, &regs);

	event = (unsigned int) wait_status >> 16;

	if (event == PTRACE_EVENT_EXEC) {
		if (ptrace(PTRACE_GETEVENTMSG, traced_process, NULL, &eventmsg) < 0) {
			perror("ptrace_geteventmsg");
			exit(EXIT_FAILURE);
		}
		
		if ((unsigned long) eventmsg != (unsigned long) traced_process) {
			if ((int)drop_pid((pid_t) eventmsg) < 0) {
				perror("drop_pid");
				exit(EXIT_FAILURE);
			}
		}
	}

	if (WIFSIGNALED(wait_status)) {
		if ((int)drop_pid(traced_process) < 0) {
			perror("drop_pid on WIFSIGNALED\n");
			exit(EXIT_FAILURE);
		}
		return true;
	}

	if (WIFEXITED(wait_status)) {
		if ((int)drop_pid(traced_process) < 0) {
			perror("drop_pid on WIFEXITED\n");
			exit(EXIT_FAILURE);
		}
		return true;
	}

	if (!WIFSTOPPED(wait_status)) {
		printf("pid %d not stopped!\n", traced_process);
		if ((int)drop_pid(traced_process) < 0) {
			perror("drop_pid on !WIFSTOPPED\n");
			exit(EXIT_FAILURE);
		}
		return true;
	}

	sig = WSTOPSIG(wait_status);

	switch (event) {
		case 0:
			if ((sig == SIGTRAP) || (sig == (SIGTRAP | 0x80))) {
				ptrace(PTRACE_GETREGS, traced_process, 0, &regs);

				if ((regs.orig_rax == SYS_mmap) && ((int)regs.r8 >= 3)) {
					if (insyscall == 0) {
						timestamp = get_timestamp();
						insyscall = 1;
					}
					else {
						memset(fname, '\0', 512 * sizeof(char));

						if (getFilepath(traced_process, (int)regs.r8, fname) == 0) {
							logical_block_addr = get_block_address(fname, regs.r9);

							if (logical_block_addr >= 0)
								create_mmap_seq(&regs, fname, m_list, m_node, timestamp);
						}
						insyscall = 0;
					}
				}

				if (ptrace_restart(PTRACE_SYSCALL, traced_process, 0) < 0)
					exit(EXIT_FAILURE);
			}
			else {
				siginfo_t si = {};
				stopped = ptrace(PTRACE_GETSIGINFO, traced_process, 0, &si) < 0;

				if (!stopped) {
					if (ptrace_restart(PTRACE_SYSCALL, traced_process, sig) < 0)
						exit(EXIT_FAILURE);
				}
				else {
					if (ptrace_restart(PTRACE_LISTEN, traced_process, 0) < 0)
						exit(EXIT_FAILURE);
				}
			}
			return true;
		case PTRACE_EVENT_EXIT:
			if (traced_process == thread_leader) {
				printf("leader exit with nprocs : %d\n", nprocs);	
				kill(traced_process, SIGUSR1);
			}
			if (ptrace_restart(PTRACE_SYSCALL, traced_process, 0) < 0)
				exit(EXIT_FAILURE);
			return true;
		case PTRACE_EVENT_STOP:
			switch (sig) {
				case SIGSTOP:
				case SIGTSTP:
				case SIGTTIN:
				case SIGTTOU:
					if (ptrace_restart(PTRACE_LISTEN, traced_process, 0) < 0)
						exit(EXIT_FAILURE);
					return true;
			}
		default:
			if (ptrace_restart(PTRACE_SYSCALL, traced_process, 0) < 0)
				exit(EXIT_FAILURE);
			return true;
	}
}

int main(int argc, char *argv[])
{
	char path[512];
	
	startup_child(argc, argv);

	while (trace());

	create_sequence(m_list, fp_mm_seq);

	fclose(fp_mm_seq);
	//fclose(fp_log);
	fclose(fp_rd_seq);
	fclose(fp_etc_seq);
	//fclose(fp_log_freq);

	printf("\n******** Every tracees have been terminated normally ********\n");
	printf("nprocs = %d\n", nprocs);

	printf("\n******** sorting start *********\n");

	strcpy(path, "/home/melody/work/trapfetch/old/script/sort.sh ");
	strcat(path, basename(argv[1]));	
	system(path);

	printf("\n******** sorting end *******\n");

	return 0;
}
