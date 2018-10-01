#include "./dstructure.h"
#include <assert.h>

#ifndef	PTRACE_EVENT_STOP
#	define PTRACE_EVENT_STOP 128
#endif

const unsigned int syscall_trap_sig = SIGTRAP | 0x80;
unsigned int ptrace_setoptions = PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | PTRACE_O_TRACECLONE | PTRACE_O_TRACEEXIT
									| PTRACE_O_TRACEEXEC | PTRACE_O_TRACESYSGOOD | PTRACE_O_EXITKILL | PTRACE_O_MMAPTRACE;
pid_t rpid;

struct _pidtab
{
	pid_t thread_leader;
	pid_t tid[100];
};

int main(int argc, char *argv[])
{
	pid_t traced_process, tid;			// Current tracee
	struct user_regs_struct regs;	// General purpose registers of tracees

	struct _pidtab pidtab[50];

	int i, j;

	siginfo_t si;
	unsigned long eventmsg;
	int wait_status;		// A wait status of tracee
	
	if (argc != 2) {	// Check weather correct or not of user's command
		printf("Usage: ./gatherer <Target app's path>\n");
		return -1;
	}
		
	if ((traced_process = fork()) == 0) {	// Child process
		if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
			perror("PTRACE_TRACEME");
			exit(-1);
		}
		//raise(SIGSTOP);
		if (execl(argv[1], argv[1], NULL) == -1) {
			perror("execl");
			exit(-1);
		}
	}
	else {	// Parent process (tracer)
		wait(&wait_status);
		
		rpid = traced_process;
		pidtab[0].thread_leader = traced_process;

		ptrace(PTRACE_SETOPTIONS, traced_process, NULL, ptrace_setoptions);
		ptrace(PTRACE_GETREGS, traced_process, NULL, &regs);
		
		ptrace(PTRACE_SYSCALL, traced_process, 0, 0); // resume execution of tracee until next system call
		
		do {	// start main loop for tracing the target application (tracee)
			traced_process = waitpid(-1, &wait_status, __WALL);

			if (traced_process < 0)
				break;

			if (WIFSTOPPED(wait_status)) {
				ptrace(PTRACE_GETREGS, traced_process, 0, &regs);
				if ((wait_status >> 8) == (SIGTRAP | PTRACE_EVENT_CLONE << 8)) {
					ptrace(PTRACE_GETEVENTMSG, traced_process, NULL, &eventmsg);
					tid = (pid_t)eventmsg;

				}
				else if ((wait_status >> 8) == (SIGTRAP | PTRACE_EVENT_FORK << 8)) {
					ptrace(PTRACE_GETEVENTMSG, traced_process, NULL, &eventmsg);
					tid = (pid_t)eventmsg;
					for (i = 0; i < 50; i++) {
						if (pidtab[i].thread_leader == 0) {
							pidtab[i].thread_leader = tid;
						}
					}
					printf("event fork [%ld]\n", tid);
				}
				else if (regs.orig_rax == __NR_mmap) {
					printf("mmap system call\n");
				}
			}
			else if (WIFEXITED(wait_status)) {	// when each tracees exited except the first tracee
				if (traced_process == rpid) {
					break;
				}
				else {
					ptrace(PTRACE_DETACH, traced_process, 0, 0);
					printf("child process %d will be terminated\n", traced_process);
					continue;
				}
			}

			ptrace(PTRACE_SYSCALL, traced_process, NULL, NULL); // resume execution untill next syscall-stop
		} while (1);
		
		printf("\n******** Every tracees have been terminated normally ********\n");
	}
	return 0;
}
