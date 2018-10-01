#include <stdio.h>
#include <libgen.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <wait.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>


#ifndef	PTRACE_EVENT_STOP
#	define PTRACE_EVENT_STOP 128
#endif

#define MAX_PIDTABSIZE	256

const unsigned int syscall_trap_sig = SIGTRAP | 0x80;
unsigned int ptrace_setoptions = PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK
									| PTRACE_O_TRACECLONE | PTRACE_O_TRACEEXIT
									| PTRACE_O_TRACEEXEC | PTRACE_O_TRACESYSGOOD;

static pid_t pidtab[256];
static unsigned int pidtabsize;
static int nprocs;
static int num_of_syscall;

void sig_handler(int signo)
{
	if (signo == SIGUSR1)
		sleep(2);

	printf("\nnum_of_syscall: %d\n", num_of_syscall);
	system("cat /proc/uptime");
}

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

	if (!err) {
		num_of_syscall++;
		return 0;
	}

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
	num_of_syscall = 0;

	if ((pid = fork()) < 0) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if (pid == 0) {
		raise(SIGSTOP);

		char *dirc = strndup(argv[1], strlen(argv[1]));
		char *dname = dirname(dirc);
	
		setenv("LD_LIBRARY_PATH", dname, 1);
		chdir(dname);

		free(dirc);

		system("cat /proc/uptime");

		if (execv(argv[1], &argv[1]) < 0) {
			perror("execv");
			exit(EXIT_FAILURE);
		}
	}

	if ((pid = waitpid(-1, &wait_status, WSTOPPED)) < 0) {
		perror("waitpid");
		exit(EXIT_FAILURE);
	}

	if (signal(SIGINT, sig_handler) == SIG_ERR) {
		printf("\ncan't catch SIGINT\n");
		exit(EXIT_FAILURE);
	}
	
	if (signal(SIGUSR1, sig_handler) == SIG_ERR) {
		printf("\ncan't catch SIGUSR1\n");
		exit(EXIT_FAILURE);
	}

	pidtab[0] = pid;
	nprocs++;

	if (ptrace_seize(pid, ptrace_setoptions) < 0)
		exit(EXIT_FAILURE);
	
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
static bool trace(void)
{
	pid_t traced_process;
	int wait_errno;
	int wait_status;
	bool stopped;
	unsigned int sig;
	unsigned int event;
	unsigned long eventmsg;
	
	pid_t pidtab[150];

	struct user_regs_struct regs;

	traced_process = waitpid(-1, &wait_status, __WALL);
	wait_errno = errno;

	if (traced_process < 0) {
		if (wait_errno == EINTR)
			return true;
		if (nprocs == 0 && wait_errno == ECHILD)
			return false;

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
			if (drop_pid((pid_t) eventmsg) < 0) {
				perror("drop_pid");
				exit(EXIT_FAILURE);
			}
		}
	}

	if (WIFSIGNALED(wait_status)) {
		if (drop_pid(traced_process) < 0) {
			perror("drop_pid on WIFSIGNALED\n");
			exit(EXIT_FAILURE);
		}
		return true;
	}

	if (WIFEXITED(wait_status)) {
		if (drop_pid(traced_process) < 0) {
			perror("drop_pid on WIFEXITED\n");
			exit(EXIT_FAILURE);
		}
		return true;
	}

	if (!WIFSTOPPED(wait_status)) {
		printf("pid %d not stopped!\n", traced_process);
		if (drop_pid(traced_process) < 0) {
			perror("drop_pid on !WIFSTOPPED\n");
			exit(EXIT_FAILURE);
		}
		return true;
	}

	sig = WSTOPSIG(wait_status);

	switch (event) {
		case 0:
			if ((sig == SIGTRAP) || (sig == (SIGTRAP | 0x80))) {
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
			}
		default:
			if (ptrace_restart(PTRACE_SYSCALL, traced_process, 0) < 0)
				exit(EXIT_FAILURE);
			return true;
	}
}

int main(int argc, char *argv[])
{

	startup_child(argc, argv);

	while (trace());

	printf("\n******** Every tracees have been terminated normally ********\n");

	return 0;
}
