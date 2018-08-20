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
#include <unistd.h>

#define PATH_BP "/home/melody/study/projects/trapfetch/logs/pf_"
#define PATH_PF "/home/melody/study/projects/trapfetch/logs/bp_"

#define PIDTABSIZE 256
#define MAX_READAHEAD 131072
#define BUFSIZE 512

// for x86_64
#if ARCH == 64
#define IP rip
#define ORIG_AX orig_rax
#define DX rdx
#define DI r8
#define TARGET_ADDR regs.rax + bp.offset
// for i386
#else
#define IP eip
#define ORIG_AX orig_eax
#define DX edx
#define DI edi
#define TARGET_ADDR regs.eax + bp.offset
#endif

#define PT_OPTIONS                                                  \
  PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEFORK | PTRACE_O_TRACEEXEC | \
      PTRACE_O_TRACEVFORK | PTRACE_O_TRACECLONE | PTRACE_O_MMAPTRACE

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

bool new_pf_list_and_node(char* buf) {}

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
  pf_list* flist;
  struct _restore_node* next;
} restore_node;

typedef struct _restore_list {
  restore_node* head;
  restore_node* tail;
} restore_list;

restore_node* new_restore_node(void* address, long long data, pf_list* flist) {
  restore_node* newnode = (restore_node*)malloc(sizeof(restore_node));
  newnode->offset = address;
  newnode->data = data;
  newnode->flist = flist;
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

extern FILE* get_fp(char* path, char* dir);