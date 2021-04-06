#define	_GNU_SOURCE

#define ARCH 64

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
#define LOG_PATH "/home/melody/work/trapfetch/logs"

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

#define	MAX_PIDTABSIZE 256

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

extern void startup_child(int argc, char **argv);
extern bool trace(void);

#ifdef MEASURE_OVERHEAD
int num_of_syscall;
#endif


#ifdef MEASURE_OVERHEAD
extern int num_of_syscall;

void sig_handler(int signo) {
  system("cat /proc/uptime");
  printf("\nnum of syscalls : %d\n", num_of_syscall);
}
#endif

size_t fnv1a_hash(const char* cp)
{
    size_t hash = 0x811c9dc5;
    while (*cp) {
        hash ^= (unsigned char) *cp++;
        hash *= 0x01000193;
    }
    return hash;
}

long long get_timestamp() {
  struct timespec ts;

  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (long long)(ts.tv_sec * pow(10, 9) + ts.tv_nsec);
}

void write_mmap_log(struct user_regs_struct *regs, char *fname) {
  size_t md;
  int prot;
  char prot_to_char;
  long long off, len, timestamp;
  void *mm_start, *mm_end;

  md = fnv1a_hash(fname);
#if ARCH == 32
  off = (long long)regs->ARGS_5 * PAGE_SIZE;
#else
  off = regs->ARGS_5;
#endif
  len = regs->ARGS_1;
  prot = (int)regs->ARGS_2;

  mm_start = (void *)regs->RET;
  mm_end = (void *)(mm_start + len);

  prot_to_char = 'r';

  if (prot & 0x4) {
    prot_to_char = 'e';
  }

  timestamp = get_timestamp();
  fprintf(fp_read, "m,%c,%s,%lld,%lld,%lld,%zu,%p,%p\n", prot_to_char, fname,
          timestamp, off, len, md, mm_start, mm_end);
}

int get_filepath(pid_t pid, int fd, char *filename) {
  char buf[512];
  
  memset(buf, '\0', 512 * sizeof(char));
  sprintf(buf, "/proc/%d/fd/%d", pid, fd);
  memset(filename, '\0', 512 * sizeof(char));

  if (readlink(buf, filename, 512 * sizeof(char)) < 0) {
    // perror("readlink");
    return -1;
  }
  return 0;
}

// create_logfile generates a log file and returns a pointer of the file.
FILE *create_logfile(char *target_name, char log_type) {
  FILE *fp;
  char fname[512];

  memset(fname, '\0', 512 * sizeof(char));

  if (log_type == 'R') {
    strcpy(fname, LOG_PATH "/r.");
  } else if (log_type == 'C') {
    strcpy(fname, LOG_PATH "/c.");
  } else {
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

pid_t lookup_pid(pid_t pid) {
  int i;

  for (i = 0; i < MAX_PIDTABSIZE; i++) {
    if (pidtab[i] == pid) return pid;
  }

  return -1;
}

pid_t alloc_new_pid(pid_t pid) {
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

pid_t drop_pid(pid_t pid) {
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

int ptrace_restart(const unsigned int op, pid_t pid, unsigned int sig) {
  int err;

  errno = 0;
  ptrace(op, pid, 0L, (unsigned long)sig);
  err = errno;

  if (!err) return 0;

  if (err == ESRCH) return 0;

  errno = err;
  perror("ptrace_restart");
  return -1;
}

int ptrace_seize(pid_t pid, unsigned int options) {
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

int ptrace_getinfo(const unsigned int op, pid_t pid, void *info) {
  int err;

  errno = 0;

  switch (op) {
    case PTRACE_GETREGS:
      ptrace(op, pid, 0, (struct user_regs_struct *)info);
    case PTRACE_GETSIGINFO:
      ptrace(op, pid, 0, (struct siginfo_t *)info);
    case PTRACE_GETEVENTMSG:
      ptrace(op, pid, 0, (unsigned long *)info);
  }

  err = errno;

  if (!err) return 0;

  perror("ptrace_getinfo");
  return -1;
}

bool trace(void) {
  pid_t tracee;
  int wait_status;

  int wait_errno;
  bool stopped;

  unsigned int sig, event;
  unsigned long eventmsg;

  struct stat fstatus;

  char fname[512];

  tracee = waitpid(-1, &wait_status, __WALL);

  if (tracee < 0) {
    wait_errno = errno;

    if (wait_errno == EINTR) return true;
    if (nprocs == 0 && wait_errno == ECHILD) {
#ifdef MEASURE_OVERHEAD
      printf("num of syscall : %d\n", num_of_syscall);
#endif
      return false;
    }

    // printf("nprocs : %d, errno : %d\n", nprocs, wait_errno);
    exit(EXIT_FAILURE);
  }

  if (lookup_pid(tracee) < 0) {
    if (alloc_new_pid(tracee) < 0) {
      perror("alloc_new_pid : pidtab is full");
      exit(EXIT_FAILURE);
    }
  }

  if (WIFSTOPPED(wait_status)) ptrace(PTRACE_GETREGS, tracee, 0, &regs);

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
  ptrace(PTRACE_GETREGS, tracee, 0, &regs);

  switch (event) {
    case 0:
      if ((sig == SIGTRAP) || (sig == SYSCALL_STOP)) {
        // ptrace_getinfo(PTRACE_GETREGS, tracee, &regs);

#ifdef MEASURE_OVERHEAD
        num_of_syscall++;
#endif
        switch (regs.ORIG_AX) {
            case SYS_mmap:
                printf("mmap at: [rip %p\n", (void *)regs.rip);
            
                // tracing only if fd value for mmap is greater than 3
                if ((int)regs.ARGS_4 < 3) {
                goto restart;
                }

                if (((int)regs.ARGS_3 & 0x20) == 0x20) {
                goto restart;
                }

                // syscall-entry stop
                if (insyscall == 0) {
                insyscall = 1;
                goto restart;
                }

                // syscall-exit stop
                insyscall = 0;
                if (get_filepath(tracee, (int)regs.ARGS_4, fname) < 0) {
                goto restart;
                }

                lstat(fname, &fstatus);
                switch (fstatus.st_mode & S_IFMT) {
                case S_IFREG:
                case S_IFLNK:
                    break;
                default:
                    goto restart;
                }

                if (fstatus.st_size == 0) {
                  goto restart;
                }

                goto restart;                            
                
            case SYS_read:;
                printf("read at: [rip %p\n", (void *)regs.rip);
                goto restart;
            case SYS_open:
                printf("open at: [rip %p\n", (void *)regs.rip);
                goto restart;
            default:
                goto restart;
        }     
      } else {
        stopped = ptrace(PTRACE_GETSIGINFO, tracee, 0, &si) < 0;

        if (!stopped) {
          if (ptrace_restart(PTRACE_SYSCALL, tracee, sig) < 0)
            exit(EXIT_FAILURE);
        } else {
          if (ptrace_restart(PTRACE_LISTEN, tracee, 0) < 0) exit(EXIT_FAILURE);
        }
        return true;
      }
    case PTRACE_EVENT_EXIT:
      if (tracee == thread_leader) {
        ;
        // printf("leader exit with nprocs : %d\n", nprocs);
      }
      goto restart;
    case PTRACE_EVENT_STOP:
      switch (sig) {
        case SIGSTOP:
        case SIGTSTP:
        case SIGTTIN:
        case SIGTTOU:
          if (ptrace_restart(PTRACE_LISTEN, tracee, 0) < 0) exit(EXIT_FAILURE);
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
void startup_child(int argc, char **argv) {
  pid_t tracee;

  int wait_status;
  char fname[512];

  char *dirc; 
  char *basec;
  char *dname; // for library path of the target application
  char *bname; // for basename of the target application
  
  // struct user_regs_struct regs;
  struct stat fstatus;

  // Create log files.
  // 'R' means read, and 'C' means candidate.
  fp_read = create_logfile(argv[1], 'R');
  fp_candidates = create_logfile(argv[1], 'C');
  if ((fp_read == NULL) || (fp_candidates == NULL)) {
    perror("Failed to log file creation");
    exit(EXIT_FAILURE);
  }

  if ((tracee = fork()) < 0) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  // tracee
  if (tracee == 0) {
    raise(SIGSTOP);

    dirc = strndup(argv[1], strlen(argv[1]));
    basec = strndup(argv[1], strlen(argv[1]));
    dname = dirname(dirc);
    bname = basename(basec);

    // set library path to target application's directory.
    setenv("LD_LIBRARY_PATH", dname, 1);

    // TARGET_PROGRAM is used in writing a log by the tracer and wrapper.
    setenv("TARGET_PROGRAM", bname, 1);
    chdir(dname);

    free(dirc);
    free(basec);
#if ARCH == 64
    setenv("LD_PRELOAD", "/home/melody/work/trapfetch/wrapper/wrapper.x86_64.so", 1);
#else
    setenv("LD_PRELOAD", "./wrapper/wrapper.i386.so", 1);
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

#ifdef MEASURE_OVERHEAD
  if (signal(SIGINT, sig_handler) == SIG_ERR) {
    printf("\n can't catch SIGINT\n");
    exit(EXIT_FAILURE);
  }
#endif

  thread_leader = tracee;

  memset(pidtab, 0, 256 * sizeof(pid_t));
  pidtab[0] = tracee;
  nprocs++;

  insyscall = 0;

  // attach ptrace() to tracee, and stop it.
  if (ptrace_seize(tracee, PT_OPTIONS) < 0) exit(EXIT_FAILURE);

  // examine tracee's current registers.
  ptrace_getinfo(PTRACE_GETREGS, tracee, &regs);

  memset(fname, '\0', 512 * sizeof(char));
  realpath(argv[1], fname);
  if (stat(fname, &fstatus) < 0) {
    perror("stat");
    exit(EXIT_FAILURE);
  }

#if ARCH == 32
  regs.RET = 0x2000000;
#else
  regs.RET = 0x400000;
#endif
  regs.ARGS_1 = fstatus.st_size;
  regs.ARGS_2 = 0x4;
  regs.ARGS_5 = 0;

  if (S_ISREG(fstatus.st_mode)) write_mmap_log(&regs, fname);

  kill(tracee, SIGCONT);
}

int main(int argc, char *argv[]) {
#ifdef MEASURE_OVERHEAD
  num_of_syscall = 0;
#endif

  startup_child(argc, argv);

  while (trace())
    ;

  printf("\ntracer: every tracees have been terminated normally\n");
  // printf("tracer: nprocs = %d\n", nprocs);

  /*
  printf("\ntracer: start sorting...\n");

  strcpy(path, "/home/melody/study/projects/trapfetch/script/sort.sh");
  strcat(path, basename(argv[1]));
  system(path);

  printf("\ntracer: sorting finished\n");
  */

  return 0;
}
