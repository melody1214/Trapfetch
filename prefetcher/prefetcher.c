#include "prefetcher.h"

FILE *fp_bp;
FILE *fp_pf;

static int pidtab[TABSIZE];
static int nprocs;

void pf_init() {
  r_list* rlist = new_restore_list();


}

void startup_child(int argc, char **argv) {
  pid_t tracee;

  int wait_status;

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

  // Open 'bp' and 'pf' files.
  fp_bp = get_fp(argv[1], PATH_BP);
  fp_pf = get_fp(argv[1], PATH_PF);

  if ((fp_bp == NULL) || (fp_pf == NULL)) {
    perror("Failed to open log files");
    exit(EXIT_FAILURE);
  }  

  tleader = tracee;

  memset(pidtab, 0, 256 * sizeof(pid_t));
  pidtab[0] = tracee;
  nprocs++;

  insyscall = 0;

  // attach ptrace() to tracee, and stop it.
  if (ptrace_seize(tracee, PT_OPTIONS) < 0) exit(EXIT_FAILURE);

  
}
