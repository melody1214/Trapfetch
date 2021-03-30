#include "prefetcher.h"

FILE *fp_bp;
FILE *fp_pf;

static int pidtab[PIDTABSIZE];
static int nprocs;
static int insyscall;
static int launched;
static int bp_counter, insert_counter;
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

void *_do_prefetch(void *list) {
  pf_list *p_list;
  pf_node *p_node;
  int fd;
  off_t offset;
  off_t length;

  p_list = (pf_list *)list;
  p_node = p_list->head;

  while (p_node->flag == 0) {
    stat(p_node->filepath, NULL);
    p_node = p_node->next;
  }

  while (p_node != NULL) {
    fd = open(p_node->filepath, O_RDONLY | O_NONBLOCK);
    offset = p_node->offset;
    length = p_node->len;

    while (1) {
      if (length > MAX_READAHEAD) {
        if (posix_fadvise(fd, offset, MAX_READAHEAD, POSIX_FADV_WILLNEED) < 0)
          perror("posix_fadvise");

        offset = offset + MAX_READAHEAD;
        length = length - MAX_READAHEAD;
      } else {
        if (posix_fadvise(fd, offset, length, POSIX_FADV_WILLNEED) < 0)
          perror("posix_fadvise");

        break;
      }
    }
    close(fd);
    p_node = p_node->next;
  }
  pthread_exit(NULL);
  return NULL;
}

int do_prefetch(pf_list *p_list) {
  pthread_t prefetch_thread;
  pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

  if (p_list == NULL) {
    perror("p_list is NULL\n");
    return -1;
  }

  if (p_list->is_prefetched == 1) {
    printf("Data has been already prefetched\n");
    return -1;
  }

  p_list->is_prefetched = 1;

#ifdef HDD
  /* Prefetching for launching with blocking when HDD is attached to computing device */
  if (launched == 0) {
    if (_do_prefetch((void *)p_list) < 0) {
      perror("_do_prefetch");
      return -1;
    }
  }
#endif

#ifdef SSD
  /* for loading */
  pthread_mutex_lock(&mtx);

  if (pthread_create(&prefetch_thread, NULL, _do_prefetch, (void *)p_list) != 0) {
    perror("pthread_create");
    return -1;
  }

  pthread_mutex_unlock(&mtx);
#endif

  return 0;
}

void pf_init(pid_t tracee, char **argv) {
  char buf[BUFSIZE];
  char path[BUFSIZE];
  long md;
  void *bp_offset;
  long off;
  long len;
  int pf_flag;
  char *basec;
  char *bname;
  char fname[512];

  offset_node *o_node;
  pf_node *p_node;
  pf_list *p_list;

  basec = strndup(argv[1], strlen(argv[1]));
  bname = basename(basec);

  memset(fname, '\0', 512 * sizeof(char));
  strcpy(fname, LOG_PATH "/bp_");
  strcat(fname, bname);

  // Open 'bp' and 'pf' files.
  fp_bp = get_fp(fname);
  
  memset(fname, '\0', 512 * sizeof(char));
  strcpy(fname, LOG_PATH "/pf_");
  strcat(fname, bname);

  fp_pf = get_fp(fname);

  if ((fp_bp == NULL) || (fp_pf == NULL)) {
    perror("Failed to open log files");
    exit(EXIT_FAILURE);
  }

  setenv("TARGET_PROGRAM", bname, 1);

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
    bp_counter++;
  }

  while (fgets(buf, BUFSIZE, fp_pf)) {
    memset(path, '\0', BUFSIZE * sizeof(char));

    sscanf(buf, "%ld,%p,%[^,],%ld,%ld,%*d,%d\n", &md, &bp_offset, path, &off, &len, &pf_flag);

    if ((p_node = new_pf_node(path, off, len, pf_flag)) == NULL) {
      perror("new_pf_node");
      exit(EXIT_FAILURE);
    }

    p_list = get_pf_list(pg_list, md, bp_offset);

    if (p_list == NULL) {
      p_list = new_pf_list(md, bp_offset);
      append_pf_list(pg_list, p_list);
    }
    
    append_pf_node(p_list, p_node);
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
  if (ptrace(PTRACE_SEIZE, pid, 0, PT_OPTIONS) < 0) {
    perror("ptrace_seize");
    return -1;
  }

  if (ptrace(PTRACE_INTERRUPT, pid, 0, 0) < 0) {
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

int restore_data(pid_t tracee, void *inst_ptr, restore_node *r_node) {
  ptrace(PTRACE_POKEDATA, tracee, inst_ptr, r_node->data);
  inst_ptr = inst_ptr - 1;
  ptrace(PTRACE_SETREGS, tracee, NULL, &regs);

  return 0;
}

long get_md_from_mmap(pid_t tracee, int di) {
  long md;
  char buf[512];
  char fname[512];

  sprintf(buf, "/proc/%d/fd/%d", tracee, di);
  memset(fname, '\0', sizeof(fname));
  readlink(buf, fname, sizeof(fname));

  md = hash(fname);

  if (md < 0) {
    perror("get_md_from_mmap");
    return -1;
  }

  return md;
}

int insert_breakpoints(long md, pid_t tracee) {
#if ARCH == 32
  unsigned long backup_data;
  unsigned long bpcode;
#else
  unsigned long long backup_data;
  unsigned long long bpcode;
#endif
  
  pf_list *p_list;
  restore_node *r_node;
  offset_node *o_node;
  void *bp_offset;
  
  o_node = get_offset_node(o_list, md);

  if (o_node == NULL) {
    perror("insert_breakpoints");
    return -1;
  }

  while (o_node != NULL) {
    bp_offset = o_node->bp_offset;
    p_list = get_pf_list(pg_list, md, bp_offset);

    if (p_list == NULL) {
      o_node = o_node->next;
      continue;
    }

    if (p_list->is_prefetched == 1) {
      o_node = o_node->next;
      continue;
    }

  /* Create and store restoring information */
#if ARCH == 32
    backup_data = ptrace(PTRACE_PEEKTEXT, tracee, (void *)(0x2000000 + bp_offset), NULL);
    r_node = new_restore_node((void *)(0x2000000 + bp_offset), backup_data, p_list);
#else
    backup_data = ptrace(PTRACE_PEEKTEXT, tracee, (void *)(0x400000 + bp_offset), NULL);
    r_node = new_restore_node((void *)(0x400000 + bp_offset), backup_data, p_list);
#endif

    append_restore_node(r_list, r_node);

#if ARCH == 32
    bpcode = (backup_data & 0xffffff00) | 0xcc;
    ptrace(PTRACE_POKETEXT, tracee, (void *)(0x2000000 + bp_offset), bpcode);
    printf("\nInsert breakpoint at: %p(offset = %p)\n", (void *)(0x2000000 + bp_offset), (void *)bp_offset);
#else
    bpcode = (backup_data & 0xffffffffffffff00) | 0xcc;
    ptrace(PTRACE_POKETEXT, tracee, (void *)(0x400000 + bp_offset), bpcode);
    printf("\nInsert breakpoint at: %p(offset = %p)\n", (void *)(0x400000 + bp_offset), (void *)bp_offset);
#endif
    insert_counter++;
    o_node = o_node->next;
  }

  return 0;
}

bool trace(void) {
  pid_t tracee;
  int wait_status;

  int wait_errno;
  bool stopped;

  unsigned int sig, event;
  unsigned long eventmsg;
  long md;

  restore_node *r_node;
  siginfo_t si = {};
  
  tracee = waitpid(-1, &wait_status, __WALL);

  if (tracee < 0) {
    wait_errno = errno;

    if (wait_errno == EINTR)
      return true;

    if (nprocs == 0 && wait_errno == ECHILD)
      return false;

    // printf("nprocs: %d, errno: %d\n", nprocs, wait_errno);
    exit(EXIT_FAILURE);
  }

  if (lookup_pid(tracee) < 0) {
    if (alloc_new_pid(tracee) < 0) {
      perror("alloc_new_pid: pidtab is full");
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
    /* insert breakpoints in application's address space */
    if (launched == 0) {
      char fname[512];
      memset(fname, '\0', sizeof(char) * sizeof(fname));
      strncpy(fname, getenv("TARGET_PROGRAM"), sizeof(fname));
      md = hash(fname);
      insert_breakpoints(md, (pid_t)eventmsg);
      launched++;
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

  /* event == 0; event_delivered_stop or ptrace_stop */
  /*  sig == 133; syscall-stop, sig == 128; SI_KERNEL */
  switch (event) {
    case 0:
      ptrace_getinfo(PTRACE_GETSIGINFO, tracee, &si);
      
      switch (si.si_code) {
        case SI_KERNEL:
#if ARCH == 32
          printf("Breakpoint[%d], EIP: 0x%p\n", bp_counter, (void *)(regs.IP - 1));
#else
          printf("Breakpoint[%d], RIP: 0x%p\n", bp_counter, (void *)(regs.IP - 1));
#endif
          r_node = get_restore_node(r_list, (void *)(regs.IP - 1));

          if (r_node == NULL) {
            perror("restore node can't be found");
            exit(EXIT_FAILURE);
          }

          if (restore_data(tracee, (void *)(regs.IP - 1), r_node) < 0) {
            perror("restore_data");
            exit(EXIT_FAILURE);
          }

          if (do_prefetch(r_node->plist) < 0) {
            perror("do_prefetch");
            exit(EXIT_FAILURE);
          }

          if (insert_counter == bp_counter) {
            ptrace_restart(PTRACE_CONT, tracee, 0);
            return true;
          }

          goto restart;
        case SYSCALL_STOP:
        case SIGTRAP:
          ptrace_getinfo(PTRACE_GETREGS, tracee, &regs);
#if ARCH == 32         
          if ((regs.ORIG_AX != SYS_mmap) || (regs.ORIG_AX != SYS_mmap2))
            goto restart;
#else
          if (regs.ORIG_AX != SYS_mmap)
            goto restart;
#endif
          if ((int)regs.ARGS_4 < 3) {
            goto restart;
          }

          if ((regs.ARGS_2 & 0x4) == 0) {
            goto restart;
          }

          // syscall-entry stop
          if (insyscall == 0) {
            insyscall = 1;
            goto restart;
          } 

          // syscall-exit stop          
          insyscall = 0;

          /* 1. Distinguish mmap() */
          md = get_md_from_mmap(tracee, (int)regs.ARGS_4);

          if (md < 0) {
            perror("get_md_from_mmap");
            exit(EXIT_FAILURE);
          }

          insert_breakpoints(md, tracee);

          goto restart;
        default:
          stopped = ptrace(PTRACE_GETSIGINFO, tracee, 0, &si) < 0;

          if (!stopped) {
            if (ptrace_restart(PTRACE_SYSCALL, tracee, sig) < 0) {
              exit(EXIT_FAILURE);
            }
          } else {
              if (ptrace_restart(PTRACE_LISTEN, tracee, 0) < 0)
                exit(EXIT_FAILURE);
          }
      }
    case PTRACE_EVENT_EXIT:
      if (tracee == thread_leader)
        goto restart;
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
  if (insert_counter == bp_counter) {
    if (ptrace_restart(PTRACE_CONT, tracee, 0) < 0) {
      perror("ptrace_restart: CONT\n");
      exit(EXIT_FAILURE);
    }
  } else {
    if (ptrace_restart(PTRACE_SYSCALL, tracee, 0) < 0) {
      perror("ptrace_restart\n");
      exit(EXIT_FAILURE);
    }
  }
  
  return true;
}

bool startup_child(int argc, char **argv) {
  pid_t tracee;

  int wait_status;
 
  if ((tracee = fork()) < 0) {
    perror("fork");
    return false;
  }

  // tracee
  if (tracee == 0) {
    raise(SIGSTOP);

    char *dirc = strndup(argv[1], strlen(argv[1]));
    char *dname = dirname(dirc);

    setenv("LD_LIBRARY_PATH", dname, 1);
    setenv("APPLICATION_PATH", argv[1], 1);
    chdir(dname);

    free(dirc);

    if (execv(argv[1], &argv[1]) < 0) {
      perror("execv");
      return false;
    }
  }

  // tracer
  if ((tracee = waitpid(-1, &wait_status, WSTOPPED)) < 0) {
    perror("waitpid");
    return false;
  }

  pf_init(tracee, argv);

  insyscall = 0;

  // attach ptrace() to tracee, and stop it.
  if (ptrace_seize(tracee) < 0)
    return false;

  /* zero values of both md and bp_offset present performing prefetching for launching application */
  if (do_prefetch(pg_list->head) < 0) {
    perror("do_prefetch");
    return false;
  }

  kill(tracee, SIGCONT);

  return true;
}
