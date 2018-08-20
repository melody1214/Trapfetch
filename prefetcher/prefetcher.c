#include "prefetcher.h"

FILE *fp_bp;
FILE *fp_pf;

static int pidtab[PIDTABSIZE];
static int nprocs;
static int insyscall;
static pid_t thread_leader;

restore_list *r_list;
offset_list *o_list;
pgroup_list *pg_list;

pid_t alloc_new_pid(pid_t pid) {
  int i;

  for (i = 0; i < PIDTABSIZE; i++) {
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

  for (i = 0; i < PIDTABSIZE; i++) {
    if (pidtab[i] == pid) {
      pidtab[i] = 0;
      nprocs--;
      return pid;
    }
  }

  return -1;
}

pid_t lookup_pid(pid_t pid) {
  int i;

  for (i = 0; i < PIDTABSIZE; i++) {
    if (pidtab[i] == pid) {
      return pid;
    }
  }

  return -1;
}

void pf_init(pid_t tracee, char **argv) {
  char buf[BUFSIZE];
  char path[BUFSIZE];
  long md;
  void *bp_offset;
  long off;
  long len;
  int pf_flag;

  offset_node *o_node;
  pf_node *p_node;
  pf_list *tmp_pf_list;

  // Open 'bp' and 'pf' files.
  fp_bp = get_fp(argv[1], PATH_BP);
  fp_pf = get_fp(argv[1], PATH_PF);

  if ((fp_bp == NULL) || (fp_pf == NULL)) {
    perror("Failed to open log files");
    exit(EXIT_FAILURE);
  }

  thread_leader = tracee;

  memset(pidtab, 0, 256 * sizeof(pid_t));
  pidtab[0] = tracee;
  nprocs++;

  // initialize all lists to hold data to prefetch
  if ((r_list = new_restore_list()) == NULL) {
    perror("new_restore_list");
    exit(EXIT_FAILURE);
  }
  if ((o_list = new_offset_list()) == NULL) {
    perror("new_offset_list");
    exit(EXIT_FAILURE);
  }

  pg_list = new_pf_group_list();

  while (fgets(buf, BUFSIZE, fp_bp)) {
    if ((o_node = new_offset_node(buf)) == NULL) {
      perror("new_offset_node");
      exit(EXIT_FAILURE);
    }
    append_offset_node(o_list, o_node);
    // total_bp_counter++;
  }

  while (fgets(buf, BUFSIZE, fp_pf)) {
    memset(path, '\0', BUFSIZE * sizeof(char));

    sscanf(buf, "%ld,%p,%[^,],%ld,%ld,%*ld,%d\n", &md, &bp_offset, path, &off,
           &len, &pf_flag);

    if ((p_node = new_pf_node(path, off, len, pf_flag)) == NULL) {
      perror("new_pf_node");
      exit(EXIT_FAILURE);
    }

    tmp_pf_list = get_pf_list(pg_list, md, bp_offset);
  }
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

int ptrace_seize(pid_t pid) {
  if (ptrace(PTRACE_SEIZE, 0, PT_OPTIONS) < 0) {
    perror("ptrace_seize");
    return -1;
  }

  if (ptrace(PTRACE_INTERRUPT, pid, 0, 0) < 0) {
    perror("ptrace_interrupt");
    return -1;
  }
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

bool startup_child(int argc, char **argv) {
  pid_t tracee;

  int wait_status;
  char buf[512];

  if ((tracee = fork()) < 0) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  // tracee
  if (tracee == 0) {
    raise(SIGSTOP);

    char *dirc = strndup(argv[1], strlen(argv[1]));
    char *dname = dirname(dirc);

    setenv("LD_LIBRARY_PATH", dname, 1);
    chdir(dname);

    free(dirc);

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

  pf_init(tracee, argv);

  insyscall = 0;

  // attach ptrace() to tracee, and stop it.
  if (ptrace_seize(tracee) < 0) exit(EXIT_FAILURE);
}
