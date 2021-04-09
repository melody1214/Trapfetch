#include "prefetcher.h"

int pidtab[PIDTABSIZE];
int nprocs;
int launched;
int total_bp_counter, bp_counter, insert_counter;
pid_t thread_leader;

restore_list *r_list;
offset_list *o_list;
pgroup_list *pg_list;

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_attr_t attr;

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

  //pthread_mutex_lock(&mtx);

  p_list = (pf_list *)list;
  p_node = p_list->head;

  printf("do prefetch for sequence [md: %zu, bp_offset: %p]\n", p_list->md, p_list->bp_offset);

#ifdef HDD
  while (p_node->flag == 0) {
    stat(p_node->filepath, NULL);
    p_node = p_node->next;
  }
#endif

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

  printf("complete prefetch for sequence [md: %zu, bp_offset: %p]\n", p_list->md, p_list->bp_offset);
  printf("thread %d will be terminated\n", gettid());

  //pthread_mutex_unlock(&mtx);

  pthread_exit((void *)0);
  
  return (void *)0;
}

int do_prefetch(pf_list *p_list) {
  pthread_t prefetch_thread;
  

  //pthread_mutex_lock(&mtx);
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

    return 0;
  }
#endif

  /* for loading */

  if (pthread_create(&prefetch_thread, &attr, _do_prefetch, (void *)p_list) != 0) {
    perror("pthread_create");
    return -1;
  }


  //pthread_attr_destroy(&attr);
  //pthread_mutex_unlock(&mtx);

  return 0;
}

void pf_init(pid_t tracee, char **argv) {
  FILE *fp_bp;
  FILE *fp_pf;
  
  char buf[BUFSIZE];
  char path[BUFSIZE];
  size_t md;
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

  if (pthread_attr_init(&attr) != 0) {
    perror("pthread_attr_init\n");
    exit(EXIT_FAILURE);
  }

  if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
    perror("pthread_attr_setdetachstate\n");
    exit(EXIT_FAILURE);
  }

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
  setenv("APP_PATH", argv[1], 1);

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

    if (o_node->bp_offset == NULL) {
      continue;
    }

    append_offset_node(o_list, o_node);
    total_bp_counter++;
  }

  while (fgets(buf, BUFSIZE, fp_pf)) {
    memset(path, '\0', BUFSIZE * sizeof(char));

    sscanf(buf, "%zu,%p,%[^,],%ld,%ld,%*d,%d\n", &md, &bp_offset, path, &off, &len, &pf_flag);

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

  fclose(fp_bp);
  fclose(fp_pf);
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

size_t get_md_from_mmap(pid_t tracee, int di) {
  size_t md;
  char buf[512];
  char fname[512];

  fflush(stdout);
  memset(buf, '\0', 512 * sizeof(char));
  sprintf(buf, "/proc/%d/fd/%d", tracee, di);
  memset(fname, '\0', 512 * sizeof(char));
  readlink(buf, fname, 512 * sizeof(char));

  md = fnv1a_hash(fname);

  if (md < 0) {
    perror("get_md_from_mmap");
    return -1;
  }

  return md;
}

int insert_breakpoints(size_t md, pid_t tracee, unsigned long long int start_off) {
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
    return -1;
  }

  while (o_node != NULL) {
    bp_offset = o_node->bp_offset;
    p_list = get_pf_list(pg_list, md, bp_offset);

    if (p_list == NULL) {
      o_node = o_node->next;
      continue;
    }

    if (get_restore_node(r_list, bp_offset) != NULL) {
      printf("[md: %zu, bp_offset: %p] Breakpoint has been already inserted\n", md, bp_offset);
      return -1;
    }

    if (p_list->is_prefetched == 1) {
      perror("This trigger has already been prefetched\n");
      return -1;
    }

    /* Create and store restoring information */
    backup_data = ptrace(PTRACE_PEEKTEXT, tracee, (void *)(start_off + bp_offset), NULL);
    r_node = new_restore_node((void *)(start_off + bp_offset), backup_data, p_list);
    append_restore_node(r_list, r_node);

#if ARCH == 32
    bpcode = (backup_data & 0xffffff00) | 0xcc;
#else
    bpcode = (backup_data & 0xffffffffffffff00) | 0xcc;
#endif

    /* Insert breakpoints into target address */
    ptrace(PTRACE_POKETEXT, tracee, (void *)(start_off + bp_offset), bpcode);
    printf("\nInsert breakpoint %d at: %p(offset = %p)\n", insert_counter, (void *)(start_off + bp_offset), bp_offset);
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
  size_t md;

  restore_node *r_node;
  
  tracee = waitpid(-1, &wait_status, __WALL);

  if (tracee < 0) {
    wait_errno = errno;

    if (wait_errno == EINTR)
      return true;

    if (nprocs == 0 && wait_errno == ECHILD)
      return false;

    printf("nprocs: %d, errno: %d\n", nprocs, wait_errno);
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
      //pthread_mutex_lock(&mtx);
      launched++;
      char fname[512];
      memset(fname, '\0', sizeof(char) * sizeof(fname));
      strncpy(fname, getenv("APP_PATH"), sizeof(fname));
      md = fnv1a_hash(fname);
#if ARCH == 32
      insert_breakpoints(md, (pid_t)eventmsg, 0x2000000);
#elif ARCH == 64
      insert_breakpoints(md, (pid_t)eventmsg, 0x400000);
#endif
      
      //pthread_mutex_unlock(&mtx);
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
    case 0:;
      siginfo_t si = {};
      ptrace_getinfo(PTRACE_GETSIGINFO, tracee, &si);
      
      switch (si.si_code) {
        case SI_KERNEL:
          //pthread_mutex_lock(&mtx);

#if ARCH == 32
          printf("Breakpoint at EIP: 0x%p\n", (void *)(regs.IP - 1));
#else
          printf("Breakpoint at RIP: 0x%p\n", (void *)(regs.IP - 1));
#endif
          
          r_node = get_restore_node(r_list, (void *)(regs.IP - 1));

          if (r_node == NULL) {
            perror("restore node can't be found");
            exit(EXIT_FAILURE);
          }
          
          /*
          if (restore_data(tracee, (void *)(regs.IP - 1), r_node) < 0) {
            perror("restore_data");
            exit(EXIT_FAILURE);
          }
          */
          
          ptrace(PTRACE_POKEDATA, tracee, regs.IP - 1, r_node->data);
          regs.IP = regs.IP - 1;
          ptrace(PTRACE_SETREGS, tracee, NULL, &regs);
          

          if (do_prefetch(r_node->plist) < 0) {
            perror("do_prefetch");
          } else {
            bp_counter++;
          }
          
          //pthread_mutex_unlock(&mtx);
          goto restart;
        case SYSCALL_STOP:
        case SIGTRAP:
          //ptrace_getinfo(PTRACE_GETREGS, tracee, &regs);
#if ARCH == 32         
          if ((regs.ORIG_AX != SYS_mmap) || (regs.ORIG_AX != SYS_mmap2))
            goto restart;
#else
          if (regs.ORIG_AX != SYS_mmap)
            goto restart;
#endif
        
          // syscall-entry stop
          if (insyscall == 0) {
            insyscall = 1;
            goto restart;
          } 

          if ((int)regs.ARGS_4 < 3) {
            goto restart;
          }

          if (((int)regs.ARGS_3 & 0x20) == 0x20) {
            goto restart;
          }

          /*
          if ((regs.ARGS_2 & 0x4) == 0) {
            goto restart;
          }
          */

          //pthread_mutex_lock(&mtx);
          
          // syscall-exit stop          
          insyscall = 0;

          /* 1. Distinguish mmap() */
          md = get_md_from_mmap(tracee, (int)regs.ARGS_4);
        
          if (md < 0) {
            perror("get_md_from_mmap");
            exit(EXIT_FAILURE);
          }
          
          if (insert_counter < total_bp_counter) {
            insert_breakpoints(md, tracee, regs.RET);
          }
          //pthread_mutex_unlock(&mtx);
          
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
          return true;
      }
    case PTRACE_EVENT_EXIT:
      if (tracee == thread_leader) {
        ;
      }
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
  //pthread_mutex_lock(&mtx);
  if (insert_counter == total_bp_counter) {
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
  //pthread_mutex_unlock(&mtx);
  //pthread_mutex_unlock(&mtx); 
 
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

    // if the target application is executed through shell script
    if (strstr(argv[1], ".sh")) {
      char *arguments[10] = { "/bin/sh", argv[1], NULL};
      if (execv(arguments[0], arguments) < 0) {
        perror("execve");
        exit(EXIT_FAILURE);
      }
    }

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

  printf("tracer start with tid: %d\n", gettid());

  pf_init(tracee, argv);

  /* zero values of both md and bp_offset present performing prefetching for launching application */
  if (do_prefetch(pg_list->head) < 0) {
    perror("do_prefetch");
    return false;
  }

  //pthread_mutex_lock(&mtx);

  insyscall = 0;

  // attach ptrace() to tracee, and stop it.
  if (ptrace_seize(tracee) < 0)
    return false;
 
  kill(tracee, SIGCONT);

  //pthread_mutex_unlock(&mtx);

  return true;
}
