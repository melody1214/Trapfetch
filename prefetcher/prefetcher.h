#include <errno.h>
#include <libgen.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define PATH_BP "/home/melody/study/projects/trapfetch/logs/pf_"
#define PATH_PF "/home/melody/study/projects/trapfetch/logs/bp_"

#define TABSIZE 256
#define MAX_READAHEAD   131072

// for x86_64
#if ARCH == 64
#define IP  rip
#define ORIG_AX orig_rax
#define DX  rdx
#define DI  r8
#define TARGET_ADDR regs.rax + bp.offset
// for i386
#else
#define IP  eip
#define ORIG_AX orig_eax
#define DX  edx
#define DI  edi
#define TARGET_ADDR regs.eax + bp.offset
#endif

unsigned int ptrace_options = PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEFORK
| PTRACE_O_TRACEEXEC | PTRACE_O_TRACEVFORK | PTRACE_O_TRACECLONE
| PTRACE_O_MMAPTRACE;
