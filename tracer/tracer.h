#define	_GNU_SOURCE

#if ARCH == 32
#define	FILE_OFFSET_BITS	64	
#endif

#define	LARGEFILE_SOURCE

#include <math.h>
#include <time.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>

#define PAGE_SIZE	4096
#define LOG_PATH(HOME)		#HOME "/work/trapfetch/logs"

#define	OPEN_FLAG		"w"
#define	OPEN_PERM		S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH

#define	SYSCALL_STOP	(SIGTRAP | 0x80)
#define	PT_OPTIONS		PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | \
						PTRACE_O_TRACECLONE | PTRACE_O_TRACEEXIT | \
						PTRACE_O_TRACEEXEC | PTRACE_O_TRACESYSGOOD | PTRACE_O_MMAPTRACE

#ifndef	PTRACE_EVENT_STOP
#define	PTRACE_EVENT_STOP	128
#endif

#if ARCH ==	32
#define	ORIG_AX	orig_eax
#define	IP		eip
#define	ARGS_0	ebx
#define	ARGS_1	ecx
#define	ARGS_2	edx
#define	ARGS_3	esi
#define	ARGS_4	edi
#define	ARGS_5	ebp
#define	RET		eax
#define	BP_ADDR	regs.eax + bp_offset

#else
#define	ORIG_AX	orig_rax
#define	IP		rip
#define	ARGS_0	rdi
#define	ARGS_1	rsi
#define	ARGS_2	rdx
#define	ARGS_3	r10
#define	ARGS_4	r8
#define	ARGS_5	r9
#define	RET		rax
#define	BP_ADDR	regs.rax + bp_offset

#endif

#define	MAX_PIDTABSIZE	256

int hash(char *filename);

struct user_regs_struct {
#if ARCH == 32
	long int ebx;
	long int ecx;
	long int edx;
	long int esi;
	long int edi;
	long int ebp;
	long int eax;
	long int xds;
	long int xes;
	long int xfs;
	long int xgs;
	long int orig_eax;
	long int eip;
	long int xcs;
	long int eflags;
	long int esp;
	long int xss;
#else
	unsigned long long int r15;
	unsigned long long int r14;
	unsigned long long int r13;
	unsigned long long int r12;
	unsigned long long int rbp;
	unsigned long long int rbx;
	unsigned long long int r11;
	unsigned long long int r10;
	unsigned long long int r9;
	unsigned long long int r8;
	unsigned long long int rax;
	unsigned long long int rcx;
	unsigned long long int rdx;
	unsigned long long int rsi;
	unsigned long long int rdi;
	unsigned long long int orig_rax;
	unsigned long long int rip;
	unsigned long long int cs;
	unsigned long long int eflags;
	unsigned long long int rsp;
	unsigned long long int ss;
	unsigned long long int fs_base;
	unsigned long long int gs_base;
	unsigned long long int ds;
	unsigned long long int es;
	unsigned long long int fs;
	unsigned long long int gs;
#endif
}regs;


pid_t pidtab[256];
pid_t thread_leader;
unsigned int pidtabsize;
int nprocs;

FILE *fp_read;
FILE *fp_candidates;
int insyscall;
siginfo_t si;
