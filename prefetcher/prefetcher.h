#include <errno.h>
#include <libgen.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

#define PATH_BP(HOME) #HOME "/work/trapfetch/logs/pf_"
#define PATH_PF(HOME) #HOME "/work/trapfetch/logs/bp_"

#define PIDTABSIZE 256
#define MAX_READAHEAD 131072
#define BUFSIZE 512

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

#ifndef PTRACE_EVENT_STOP
#define PTRACE_EVENT_STOP 128
#endif

#define SYSCALL_STOP  (SIGTRAP | 0x80)

#define PT_OPTIONS                                                  \
  PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEFORK | PTRACE_O_TRACEEXEC | \
      PTRACE_O_TRACEVFORK | PTRACE_O_TRACECLONE | PTRACE_O_MMAPTRACE


extern FILE* get_fp(char* path, char* dir);
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

typedef struct _pf_node {
  char filepath[512];
  long long offset;
  long long len;
  int flag;
  struct _pf_node* next;
} pf_node;

typedef struct _pf_list {
  long md;
  void* bp_offset;
  int is_prefetched;
  pf_node* head;
  pf_node* tail;
  struct _pf_list* next;
} pf_list;

typedef struct pf_group_list {
  pf_list* head;
  pf_list* tail;
} pgroup_list;

pf_node* new_pf_node(char* filepath, long long offset, long long len,
                     int flag) {
  pf_node* newnode = (pf_node*)malloc(sizeof(pf_node));
  strncpy(newnode->filepath, filepath, sizeof(newnode->filepath));
  newnode->offset = offset;
  newnode->len = len;
  newnode->flag = flag;
  newnode->next = NULL;
  return newnode;
}

pf_list* new_pf_list(long md, void* bp_offset) {
  pf_list* newlist = (pf_list*)malloc(sizeof(pf_list));
  newlist->md = md;
  newlist->bp_offset = bp_offset;
  newlist->is_prefetched = 0;
  newlist->head = NULL;
  newlist->tail = NULL;
  newlist->next = NULL;
  return newlist;
}

pgroup_list* new_pf_group_list() {
  pgroup_list* newlist = (pgroup_list*)malloc(sizeof(pgroup_list));
  newlist->head = NULL;
  newlist->tail = NULL;
  return newlist;
}

void append_pf_node(pf_list* list, pf_node* node) {
  if (list->head == NULL) {
    list->head = node;
    list->tail = node;
  } else {
    list->tail->next = node;
    list->tail = node;
  }
}

void append_pf_list(pgroup_list* list, pf_list* item) {
  if (list->head == NULL) {
    list->head = item;
    list->tail = item;
  } else {
    list->tail->next = item;
    list->tail = item;
  }
}

pf_list* get_pf_list(pgroup_list* list, long md, void* bp_offset) {
  pf_list* temp = list->head;
  while (temp != NULL) {
    if (temp->md == md) {
      if (temp->bp_offset == bp_offset) {
        return temp;
      }
    }
    temp = temp->next;
  }
  return NULL;
}

typedef struct _restore_node {
  void* offset;
  long long data;
  pf_list* plist;
  struct _restore_node* next;
} restore_node;

typedef struct _restore_list {
  restore_node* head;
  restore_node* tail;
} restore_list;

restore_node* new_restore_node(void* address, long long data, pf_list* plist) {
  restore_node* newnode = (restore_node*)malloc(sizeof(restore_node));
  newnode->offset = address;
  newnode->data = data;
  newnode->plist = plist;
  newnode->next = NULL;
  return newnode;
}

restore_list* new_restore_list() {
  restore_list* newlist = (restore_list*)malloc(sizeof(restore_list));
  newlist->head = NULL;
  newlist->tail = NULL;
  return newlist;
}

void append_restore_node(restore_list* list, restore_node* node) {
  if (list->head == NULL) {
    list->head = node;
    list->tail = node;
  } else {
    list->tail->next = node;
    list->tail = node;
  }
}

restore_node* get_restore_node(restore_list* list, void* key) {
  restore_node* temp = list->head;
  while (temp != NULL) {
    if ((temp->offset) == key) {
      return temp;
    }
    temp = temp->next;
  }
  return NULL;
}

typedef struct _offset_node {
  long md;
  void* bp_offset;
  struct _offset_node* next;
} offset_node;

typedef struct _offset_list {
  offset_node* head;
  offset_node* tail;
} offset_list;

offset_node* new_offset_node(char* buf) {
  offset_node* newnode = (offset_node*)malloc(sizeof(offset_node));

  sscanf(buf, "%ld%p\n", &newnode->md, &newnode->bp_offset);
  newnode->next = NULL;
  return newnode;
}

offset_list* new_offset_list() {
  offset_list* newlist = (offset_list*)malloc(sizeof(offset_list));
  newlist->head = NULL;
  newlist->tail = NULL;
  return newlist;
}

void append_offset_node(offset_list* o_list, offset_node* o_node) {
  if (o_list->head == NULL) {
    o_list->head = o_node;
    o_list->tail = o_node;
  } else {
    o_list->tail->next = o_node;
    o_list->tail = o_node;
  }
}

offset_node* get_offset_node(offset_list* o_list, long md) {
  offset_node* o_node = o_list->head;
  // printf("\nKEY : %d\n", md);
  while (o_node != NULL) {
    // printf("[SEARCH]= %d, %llx, %p\n", o_node->md, o_node->bp_offset,
    // o_node->next);
    if ((o_node->md) == md) {
      return o_node;
    }
    o_node = o_node->next;
  }
  return NULL;
}