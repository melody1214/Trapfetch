//#define	_GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>  //for Using basename
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // memcpy()
#include <sys/ptrace.h>
#include <sys/stat.h>
#include <sys/syscall.h>  //SYS_mmap variable
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sched.h>
#include "../../header/hash.h"
#include "../../header/structures.h"

#define BUFFER_SIZE 256
#define PREFETCH_AHEAD 1
#define FADVISE_LIMIT 131072

/* x64 */
#if ARCH == 64
#define IP rip
#define ORIG_AX orig_rax
#define DX rdx
#define DI r8
#define TARGET_ADDRESS regs.rax + bp_offset

/* x86 */
#else
#define IP eip
#define ORIG_AX orig_eax
#define DX edx
#define DI edi
#define TARGET_ADDRESS regs.eax + bp_offset
#endif

#define TABSIZE 256
static int pidtab[TABSIZE];
static int nprocs;
/*
typedef struct _pf_files {
        int fd;
        off_t offset;
        off_t length;
} pf_files_;
*/
// pf_files_ pf_files[2048] = {0, 0, 0};

// char *orig_path;

// int status;
static int bp_counter = 1;
static int insert_counter = 0;
static int total_bp_counter = 0;

static unsigned int ptrace_options =
    PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEFORK | PTRACE_O_TRACEEXEC |
    PTRACE_O_TRACEVFORK | PTRACE_O_TRACECLONE | PTRACE_O_MMAPTRACE;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

#define PTRACE_EVENT_STOP 128

struct user_regs_struct {
#if ARCH == 64
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
#else
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
#endif
};

void* do_prefetch(void* prefetch_flist) {
  f_list* flist = (f_list*)prefetch_flist;
  f_node* temp_node = flist->head;

  while (temp_node->flag == 0) {
    stat(temp_node->filepath, NULL);
    temp_node = temp_node->next;
  }

  while (temp_node != NULL) {
    int temp_fd = open(temp_node->filepath, O_RDONLY | O_NONBLOCK);
    off_t starting_offset = temp_node->offset;
    off_t length = temp_node->len;

    while (1) {
      if (length > FADVISE_LIMIT) {
        if (posix_fadvise(temp_fd, starting_offset, FADVISE_LIMIT,
                          POSIX_FADV_WILLNEED) < 0) {
          perror("posix_fadvise");
        }
        starting_offset = starting_offset + FADVISE_LIMIT;
        length = length - FADVISE_LIMIT;
      } else {
        if (posix_fadvise(temp_fd, starting_offset, length,
                          POSIX_FADV_WILLNEED) < 0) {
          perror("posix_fadvise");
        }
        break;
      }
    }
    close(temp_fd);
    temp_node = temp_node->next;
  }

  // pthread_mutex_unlock(&mtx);
  // free(orig_path);
  pthread_exit(NULL);

  return NULL;
}

/*
if (pthread_attr_init(&attr) < 0) {
        perror("pthread_attr_init");
}

if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) < 0) {
        perror("pthread_attr_setdetachstate");
}

if (pthread_create(&prefetch_thread, &attr, do_prefetch, (void*)prefetch_list) <
0) { perror("pthread_create");
}

if (pthread_attr_destroy(&attr) < 0) {
        perror("pthread_attr_destroy");
}
*/

int main(int argc, char* argv[]) {
  struct user_regs_struct regs;

  pthread_t prefetch_thread;
  pthread_attr_t attr;
#if ARCH == 64
  unsigned long long backup_data = 0;
  unsigned long long bpcode;
  void* bp_offset;
#else
  unsigned long backup_data = 0;
  unsigned long bpcode;
  void* bp_offset;
#endif
  unsigned int event;
  unsigned long eventmsg;

  int insyscall = 0;
  int wait_status;
  int wait_errno;

  void* res;

  int i;
/*
	cpu_set_t set;

	CPU_ZERO(&set);
	CPU_SET(0, &set);
	sched_setaffinity(0, sizeof(cpu_set_t), &set);
*/

  /* Initialize pidtab */
  for (i = 0; i < TABSIZE; i++) {
    pidtab[i] = 0;
  }
  nprocs = 0;

  char* app_name = basename(argv[1]);

  r_list* rlist = newRList();
  offset_list* o_list = newOffsetList();

  if (argc < 2) {
    printf("Usage ./caller <target>\n");
    exit(0);
  }

  int cid = fork();
  /* Child process */
  if (cid == 0) {
    raise(SIGSTOP);

    char* dirc = strdup(argv[1]);
    setenv("LD_LIBRARY_PATH", dirname(dirc), 1);

    if (execv(argv[1], &argv[1]) < 0) {
      perror("execl");
      exit(-1);
    }
  } else {
    int tracee_pid;

    char* orig_path = "";
    // res = (int *)malloc(sizeof(int));
    //*res = 0;
    tracee_pid = waitpid(-1, &wait_status, WSTOPPED);
    pidtab[0] = tracee_pid;
    nprocs++;

    char bp_prefix[BUFFER_SIZE] = "/home/shared/prefetch/log/bp_";
    char pf_prefix[BUFFER_SIZE] = "/home/shared/prefetch/log/pf_";

    r_list* rlist = newRList();

    // Load bp_log for Distinguish MMAP
    FILE* bp_file = fopen(strcat(bp_prefix, app_name), "r");
    if (bp_file == NULL) {
      perror("fopen");
      exit(-1);
    }
    char bp_read_buffer[BUFFER_SIZE];

    while (fgets(bp_read_buffer, BUFFER_SIZE, bp_file)) {
      long bp_md;
#if ARCH == 64
      void* bp_bp_offset;
      sscanf(bp_read_buffer, "%ld,%p\n", &bp_md, &bp_bp_offset);
#else
      void* bp_bp_offset;
      sscanf(bp_read_buffer, "%ld,%p\n", &bp_md, &bp_bp_offset);
#endif
      offset_node* o_node = newOffsetNode(bp_md, bp_bp_offset);
      appendONode(o_list, o_node);
      total_bp_counter++;
    }

    // Load prefetch Sequence for Prefetch
    fl_list* fllist = newFLlist();
    FILE* pf_file = fopen(strcat(pf_prefix, app_name), "r");
    if (pf_file == NULL) {
      perror("fopen");
      exit(-1);
    }
    char pf_read_buffer[BUFFER_SIZE];
    while (fgets(pf_read_buffer, BUFFER_SIZE, pf_file)) {
      long pf_md;
      char pf_filepath[512];
      long long pf_offset;
      long pf_len;
      int pf_flag;
      memset(pf_filepath, '\0', 512 * sizeof(char));
#if ARCH == 64
      void* pf_bp_offset;
      sscanf(pf_read_buffer, "%ld,%p,%[^,],%lld,%ld,%*[^,],%d\n", &pf_md,
             &pf_bp_offset, pf_filepath, &pf_offset, &pf_len, &pf_flag);
#else
      void* pf_bp_offset;
      sscanf(pf_read_buffer, "%ld,%p,%[^,],%lld,%ld,%*[^,],%d\n", &pf_md,
             &pf_bp_offset, pf_filepath, &pf_offset, &pf_len, &pf_flag);
#endif

      f_node* new_fnode = newFNode(pf_filepath, pf_offset, pf_len, pf_flag);
      f_list* temp_flist = getFList(fllist, pf_md, pf_bp_offset);
      if (temp_flist == NULL) {
        f_list* new_flist = newFList(pf_md, pf_bp_offset);
        appendFNode(new_flist, new_fnode);
        appendFList(fllist, new_flist);
      } else {
        appendFNode(temp_flist, new_fnode);
      }
    }

    /* Launch Prefetch */
    // f_list* lauch_prefetch = getFList(fllist, 0, 0x0);
    f_list* lauch_prefetch = fllist->head;
    // long prefetched_md = lauch_prefetch->md;
    // void *prefetched_bp = lauch_prefetch -> bp_offset;
    if ((lauch_prefetch != NULL) && ((lauch_prefetch->is_prefetched) == 0)) {
      f_node* temp_node = lauch_prefetch->head;

      while (temp_node->flag == 0) {
        stat(temp_node->filepath, NULL);
        temp_node = temp_node->next;
      }

      // temp_node = lauch_prefetch->head;
      while (temp_node != NULL) {
        int temp_fd = open(temp_node->filepath, O_RDONLY | O_NONBLOCK);
        off_t starting_offset = temp_node->offset;
        off_t length = temp_node->len;

        while (1) {
          if (length > FADVISE_LIMIT) {
            if (posix_fadvise(temp_fd, starting_offset, FADVISE_LIMIT,
                              POSIX_FADV_WILLNEED) < 0) {
              perror("posix_fadvise");
            }
            starting_offset = starting_offset + FADVISE_LIMIT;
            length = length - FADVISE_LIMIT;
          } else {
            if (posix_fadvise(temp_fd, starting_offset, length,
                              POSIX_FADV_WILLNEED) < 0) {
              perror("posix_fadvise");
            }
            break;
          }
        }
        close(temp_fd);
        temp_node = temp_node->next;
      }

      lauch_prefetch->is_prefetched = 1;
    }

    if (ptrace(PTRACE_SEIZE, tracee_pid, 0, ptrace_options) < 0) {
      perror("ptrace_seize");
      exit(EXIT_FAILURE);
    }
    if (ptrace(PTRACE_INTERRUPT, tracee_pid, 0L, 0L) < 0) {
      perror("ptrace_inerrupt");
      exit(EXIT_FAILURE);
    }

    system("cat /proc/uptime");

    if (kill(tracee_pid, SIGCONT) < 0) {
      perror("kill(SIGCONT)");
      exit(EXIT_FAILURE);
    }

    /* Start Tracing */
    int launched = 0;
    while (1) {
      tracee_pid = waitpid(-1, &wait_status, __WALL);

      wait_errno = errno;

      if (tracee_pid < 0) {
        if (wait_errno == EINTR) continue;
        if (nprocs == 0 && wait_errno == ECHILD) break;

        errno = wait_errno;
        perror("waitpid");
        printf("nprocs : %d, errno : %d\n", nprocs, wait_errno);
        exit(EXIT_FAILURE);
      }

      /* Find old pidtab */
      for (i = 0; i < TABSIZE; i++) {
        if (pidtab[i] == tracee_pid) break;
      }
      /* insert new pid */
      if (i == TABSIZE) {
        for (i = 0; i < TABSIZE; i++) {
          if (pidtab[i] == 0) {
            pidtab[i] = tracee_pid;
            nprocs++;
            break;
          }
        }
        if (i == TABSIZE) {
          printf("tab is full\n");
          exit(-1);
        }
      }

      /* GET a data */
      event = (unsigned int)wait_status >> 16;
      ptrace(PTRACE_GETREGS, tracee_pid, NULL, &regs);

      if (event == PTRACE_EVENT_EXEC) {
        if (ptrace(PTRACE_GETEVENTMSG, tracee_pid, NULL, &eventmsg) == 0) {
          if ((unsigned long)eventmsg != (unsigned long)tracee_pid) {
            if (pidtab[i] == (int)eventmsg) {
              /* del tracee_pid in pidtab */
              for (i = 0; i < TABSIZE; i++) {
                if (pidtab[i] == tracee_pid) {
                  pidtab[i] = 0;
                  nprocs--;
                }
              }
              break;
            }
          }
        } else {
          perror("PTRACE_GETEVENTMSG");
          break;
        }

        if (launched == 0) {
          /* Direct breakpoint insert */
          char fname[512];
          memset(fname, '\0', 512 * sizeof(char));
          realpath(argv[1], fname);
          long application_md = hash(fname);
          offset_node* temp_onode = getONode(o_list, application_md);
          while (temp_onode != NULL) {
            bp_offset = temp_onode->bp_offset;

            f_list* suitable_flist =
                getFList(fllist, application_md, bp_offset);
            if (suitable_flist != NULL && suitable_flist->is_prefetched == 1) {
              temp_onode = temp_onode->next;
              continue;
            }

#if ARCH == 64
            backup_data = ptrace(PTRACE_PEEKTEXT, (pid_t)eventmsg,
                                 (void*)(0x400000 + bp_offset), NULL);
#else
            backup_data = ptrace(PTRACE_PEEKTEXT, (pid_t)eventmsg,
                                 (void*)(0x2000000 + bp_offset), NULL);
#endif
            // f_list* suitable_flist = getFList(fllist, application_md,
            // bp_offset);
            if (suitable_flist != NULL) {
              /* 3.Make restore data structure */
#if ARCH == 64
              r_node* rnode = newRNode((void*)(0x400000 + bp_offset),
                                       backup_data, suitable_flist);
#else
              r_node* rnode = newRNode((void*)(0x2000000 + bp_offset),
                                       backup_data, suitable_flist);
#endif
              appendRNode(rlist, rnode);
              /* 4. Insert the breakpoint */
#if ARCH == 64
              bpcode = (backup_data & 0xffffffffffffff00) | 0xcc;
              ptrace(PTRACE_POKETEXT, (pid_t)eventmsg,
                     (void*)(0x400000 + bp_offset), bpcode);
              printf("\nINSERT BP : %p(offset = %p)\n",
                     (void*)(0x400000 + bp_offset), (void*)bp_offset);
#else
              bpcode = (backup_data & 0xffffff00) | 0xcc;
              ptrace(PTRACE_POKETEXT, (pid_t)eventmsg,
                     (void*)(0x2000000 + bp_offset), bpcode);
              printf("\nINSERT BP : %p(offset = %p)\n",
                     (void*)(0x2000000 + bp_offset), (void*)bp_offset);
#endif
              insert_counter++;
            }
            temp_onode = temp_onode->next;
          }
          launched++;
        }
      }

      /* tracee receive exit (or some signals?) */
      if (WIFSIGNALED(wait_status) || WIFEXITED(wait_status)) {
        for (i = 0; i < TABSIZE; i++) {
          if (pidtab[i] == tracee_pid) {
            pidtab[i] = 0;
            nprocs--;
            break;
          }
        }
        continue;
      }
      /* EXECPTION */
      if (!WIFSTOPPED(wait_status)) {
        printf("pid %d is not stopped.\n", tracee_pid);
        for (i = 0; i < TABSIZE; i++) {
          if (pidtab[i] == tracee_pid) {
            pidtab[i] = 0;
            nprocs--;
            break;
          }
        }
        continue;
      }

      int sig = WSTOPSIG(wait_status);

      /*  event == 0; event_delivered_stop or ptrace_stop */
      if (event == 0) {
        /* sig == 133; syscall
         *     == 128; SI_KERNEL
         *     == else; */
        siginfo_t si = {};
        ptrace(PTRACE_GETSIGINFO, tracee_pid, 0, &si);
        if (si.si_code == SI_KERNEL) {
#if ARCH == 64
          printf("BreakPoint[%d], rip-1 = 0x%llx\n", bp_counter, regs.IP - 1);
#else
          printf("BreakPoint[%d], eip-1 = 0x%lx\n", bp_counter, regs.IP - 1);
#endif

          // pthread_mutex_lock(&mtx);
          bp_counter++;
          r_node* temp_rnode = getRNode(rlist, (void*)(regs.IP - 1));

          /* Restore */
          if (temp_rnode != NULL) {
            ptrace(PTRACE_POKEDATA, tracee_pid, regs.IP - 1, temp_rnode->data);
            regs.IP = regs.IP - 1;
            ptrace(PTRACE_SETREGS, tracee_pid, NULL, &regs);
          } else {
            printf("Restore data Not found\n");
          }

          if (temp_rnode != NULL) {
            f_list* prefetch_list = temp_rnode->flist;

            if (prefetch_list->is_prefetched == 0) {
              pthread_mutex_lock(&mtx);

              prefetch_list->is_prefetched = 1;
              /*
              if (pthread_attr_init(&attr) < 0) {
                      perror("pthread_attr_init");
              }

              if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) <
              0) { perror("pthread_attr_setdetachstate");
              }
              */

              if (pthread_create(&prefetch_thread, NULL, do_prefetch,
                                 (void*)prefetch_list) != 0) {
                perror("pthread_create");
              }
              /*
              if (pthread_join(prefetch_thread, NULL) != 0) {
                      perror("pthread_join");
              }
              */
              // pthread_exit(&prefetch_thread);

              /*
              if (pthread_attr_destroy(&attr) < 0) {
                      perror("pthread_attr_destroy");
              }
              */
              // do_prefetch(prefetch_list);
              pthread_mutex_unlock(&mtx);
              /*
              pthread_attr_t attr;

              if (pthread_attr_init(&attr) < 0) {
                      perror("pthread_attr_init");
              }

              if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) <
              0) { perror("pthread_attr_setdetachstate");
              }

              if (pthread_create(&prefetch_thread, &attr, do_prefetch,
              (void*)prefetch_list) < 0) { perror("pthread_create");
              }

              if (pthread_attr_destroy(&attr) < 0) {
                      perror("pthread_attr_destroy");
              }
              */
              // pthread_mutex_lock(&mtx);

              // do_thread((void *)prefetch_list);
              // pthread_mutex_lock(&mtx);
              /*
              if (pthread_create(&prefetch_thread, NULL, do_prefetch,
              (void*)prefetch_list) != 0) { perror("pthread_create");
              }


              if (pthread_join(prefetch_thread, NULL) != 0){
                      perror("pthread_join");
              }
              else {
                      pthread_exit(&res);
                      pthread_mutex_unlock(&mtx);
              }
              */

              // for (int i = 0; i < PREFETCH_AHEAD; i++) {

              // pthread_attr_t attr;
              /*
              if (pthread_attr_init(&attr) < 0) {
                      perror("pthread_attr_init");
              }
              if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) <
              0) { perror("pthread_attr_setdetachstate");
              }
              */
              // pthread_mutex_unlock(&mtx);
              /* old thread code.
              if ( pthread_detach(prefetch_thread) < 0) {
                      perror("pthread_detach");
              }
              */
              /*
              if (pthread_attr_destroy(&attr) < 0) {
                      perror("pthread_attr_destory");
              }
              */

              // do_prefetch(prefetch_list);
              /*
              prefetch_list = prefetch_list->next;
              if (prefetch_list == NULL) {
                      break;
              }
              */

              //}

            } else {
              printf("pf list not found!\n");
            }
          }

          if (insert_counter == total_bp_counter) {
            ptrace(PTRACE_CONT, tracee_pid, 0, 0);
            continue;
          } else {
            ptrace(PTRACE_SYSCALL, tracee_pid, 0, 0);
            continue;
          }
        } else if ((si.si_code == (SIGTRAP | 0x80)) ||
                   (si.si_code == SIGTRAP)) {
#if ARCH == 64
          if (regs.ORIG_AX == SYS_mmap) {
#else
          if ((regs.ORIG_AX == SYS_mmap) || (regs.ORIG_AX == SYS_mmap2)) {
#endif
            if (insyscall == 0) {
              insyscall = 1;
            } else {
              /* Catch MMAP with PROC_EXEC */
              if (((regs.DX & 0x4) != 0) && (regs.DI >= 3)) {
                /* 1. Distingush mmap() */
                if (total_bp_counter > insert_counter) {
                  char buf[512], fname[512];
                  sprintf(buf, "/proc/%d/fd/%d", tracee_pid, (int)regs.DI);
                  memset(fname, '\0', sizeof(fname));
                  readlink(buf, fname, sizeof(fname));
                  long md = hash(fname);
                  offset_node* temp_onode = getONode(o_list, md);

                  while (temp_onode != NULL) {
                    bp_offset = temp_onode->bp_offset;

                    f_list* suitable_flist = getFList(fllist, md, bp_offset);

                    if (suitable_flist != NULL &&
                        suitable_flist->is_prefetched == 1) {
                      temp_onode = temp_onode->next;
                      continue;
                    }

                    backup_data = ptrace(PTRACE_PEEKTEXT, tracee_pid,
                                         TARGET_ADDRESS, NULL);

                    // f_list* suitable_flist = getFList(fllist, md, bp_offset);
                    if (suitable_flist != NULL) {
                      /* 3.Make restore data structure */
                      r_node* rnode =
                          newRNode(TARGET_ADDRESS, backup_data, suitable_flist);
                      appendRNode(rlist, rnode);
                      /* 4. Insert the breakpoint */
#if ARCH == 64
                      bpcode = (backup_data & 0xffffffffffffff00) | 0xcc;
#else
                      bpcode = (backup_data & 0xffffff00) | 0xcc;
#endif
                      ptrace(PTRACE_POKETEXT, tracee_pid, TARGET_ADDRESS,
                             bpcode);
                      printf("\nINSERT BP : %p(offset = %p)\n", TARGET_ADDRESS,
                             bp_offset);
                      insert_counter++;
                    }
                    temp_onode = temp_onode->next;
                  }
                }
              }
              insyscall = 0;
            }
          }
          if (insert_counter == total_bp_counter) {
            ptrace(PTRACE_CONT, tracee_pid, 0, 0);
            continue;
          } else {
            if (ptrace(PTRACE_SYSCALL, tracee_pid, 0, 0) < 0) {
              perror("in syscall, PTRACE_SYSCALL");
              continue;
            }
          }
        } else {
          siginfo_t si = {};
          bool stopped = ptrace(PTRACE_GETSIGINFO, tracee_pid, 0, &si) < 0;
          if (!stopped) {
            if (insert_counter == total_bp_counter) {
              ptrace(PTRACE_CONT, tracee_pid, 0, sig);
              continue;
            } else {
              ptrace(PTRACE_SYSCALL, tracee_pid, 0, sig);
              continue;
            }
          } else {
            ptrace(PTRACE_LISTEN, tracee_pid, 0, 0);
            continue;
          }
        }
      } else if (event == PTRACE_EVENT_EXIT) {
        if (insert_counter == total_bp_counter) {
          ptrace(PTRACE_CONT, tracee_pid, 0, 0);
          continue;
        } else {
          ptrace(PTRACE_SYSCALL, tracee_pid, 0, 0);
          continue;
        }
      } else if (event == PTRACE_EVENT_STOP) {
        if ((sig == SIGSTOP) || (sig == SIGTSTP) || (sig == SIGTTIN) ||
            (sig == SIGTTOU)) {
          ptrace(PTRACE_LISTEN, tracee_pid, 0, 0);
          continue;
        }
      }
      if (insert_counter == total_bp_counter) {
        ptrace(PTRACE_CONT, tracee_pid, 0, 0);
        continue;
      } else {
        ptrace(PTRACE_SYSCALL, tracee_pid, 0, 0);
        continue;
      }
    }
  }
  return 0;
}
