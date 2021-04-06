#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "hashmap.h"

#define IS_BURST 1

#define READ 1
#define MMAP 2

#define OFFTOLEN(n) (n)->off + (n)->len + 1

#define SUBSET(a, b) ((b)->off >= (a)->off) && ((OFFTOLEN(b)) <= (OFFTOLEN(a)))
#define EXPAND(a, b)                                     \
  ((b)->off >= (a)->off) && ((b)->off <= OFFTOLEN(a)) && \
      OFFTOLEN(b) > OFFTOLEN(a)

extern size_t fnv1a_hash(char *input);

typedef struct _read_node {
  char path[512];
  long long ts;
  long long off;
  long long len;
  long lba;
  int ino;
  struct _read_node *next;
} read_node;

#ifndef _ANALYZER_HEADER_INCLUDED
#define _ANALYZER_HEADER_INCLUDED

typedef struct _meta_node {
  read_node *ptr;
  struct _meta_node *next;
} meta_node;

typedef struct _read_list {
  read_node *head;
  read_node *tail;
  meta_node *meta_head;
  meta_node *meta_tail;
  long long start_ts;
  long long end_ts;
  bool is_burst;
  void *bp_offset;
  size_t md;
  struct _read_list *next;
} read_list;

typedef struct _pf_list {
  read_list *head;
  read_list *tail;
} pf_list;

typedef struct _mm_node {
  char path[512];
  long long ts;
  long long off;
  long long len;
  size_t md;
  void *start_addr;
  void *end_addr;
  struct _mm_node *next;
} mm_node;

typedef struct _mm_list {
  mm_node *head;
  mm_node *tail;
} mm_list;

typedef struct trigger_node {
  void *ret_addr;
  long long ts;
  struct trigger_node *next;
} trigger_t;

typedef struct {
  trigger_t *head;
  trigger_t *tail;
} trigger_list;


typedef struct {
  char key[64];
  long long ts;
} hashmap_t;

read_list *init_read_list() {
  read_list *newlist = (read_list *)malloc(sizeof(read_list));

  newlist->head = NULL;
  newlist->tail = NULL;
  newlist->next = NULL;

  newlist->bp_offset = NULL;
  newlist->md = 0;
  newlist->meta_head = NULL;
  newlist->meta_tail = NULL;

  return newlist;
}

mm_list *init_mm_list() {
  mm_list *newlist = (mm_list *)malloc(sizeof(mm_list));

  newlist->head = NULL;
  newlist->tail = NULL;

  return newlist;
}

pf_list *init_pf_list() {
  pf_list *newlist = (pf_list *)malloc(sizeof(pf_list));

  newlist->head = NULL;
  newlist->tail = NULL;

  return newlist;
}

meta_node *new_meta_node(read_node *n) {
  meta_node *newnode;

  newnode = (meta_node *)malloc(sizeof(meta_node));

  newnode->ptr = n;
  newnode->next = NULL;

  return newnode;
}

read_node *new_read_node(char *buf, int type) {
  read_node *newnode;
  newnode = (read_node *)malloc(sizeof(read_node));

  switch (type) {
    case READ:
      sscanf(buf, "%*[^,],%[^,],%lld,%lld,%lld", newnode->path, &newnode->ts,
             &newnode->off, &newnode->len);
      break;
    case MMAP:
      sscanf(buf, "%*[^,],%*[^,],%[^,],%lld,%lld,%lld,%*[^,],%*[^,],%*[^,]", newnode->path, &newnode->ts,
             &newnode->off, &newnode->len);
      break;
    default:
      break;
  }

  newnode->next = NULL;

  return newnode;
}

mm_node *new_mmap_node(char *buf) {
  char fname[512];

  mm_node *newnode = (mm_node *)malloc(sizeof(mm_node));
  memset(newnode->path, '\0', sizeof(newnode->path));
  memset(fname, '\0', sizeof(fname));

  sscanf(buf, "%*[^,],%*[^,],%[^,],%lld,%lld,%lld,%*[^,],%p,%p", fname, &newnode->ts,
         &newnode->off, &newnode->len, &newnode->start_addr,
         &newnode->end_addr);

  strncpy(newnode->path, fname, sizeof(newnode->path));

  newnode->next = NULL;

  return newnode;
}

void insert_node_into_mm_list(mm_list *l, mm_node *n) {
  mm_node *tmp = l->head;

  if (tmp == NULL) {
    l->head = n;
    l->tail = n;
    return;
  }

  while (tmp->next != NULL) {
    tmp = tmp->next;
  }

  tmp->next = n;
  l->tail = n;
}

void insert_meta_node_into_read_list(read_list *rl, meta_node *i) {
  meta_node *n = rl->meta_head;
  meta_node *tmp;

  if (n == NULL) {
    rl->meta_head = i;
    rl->meta_tail = i;
    return;
  }

  while (n->next != NULL) {
    if (n->ptr->ino < i->ptr->ino) {
      n = n->next;
      continue;
    }

    if (n->ptr->ino == i->ptr->ino) {
      free(i);
      return;
    }

    if (n->ptr->ino > i->ptr->ino) {
      break;
    }
  }

  if (n->ptr->ino < i->ptr->ino) {
    if (n->next == NULL) {
      rl->meta_tail = i;
    }
    n->next = i;
    return;
  }

  if (n->ptr->ino == i->ptr->ino) {
    free(i);
    return;
  }

  if (n->ptr->ino > i->ptr->ino) {
    tmp = (meta_node *)malloc(sizeof(meta_node));

    memcpy(tmp, n, sizeof(meta_node));
    memcpy(n, i, sizeof(meta_node));

    n->next = tmp;

    if (tmp->next == NULL) {
      rl->meta_tail = tmp;
    }

    free(i);
  }
}

void insert_node_into_read_list(read_list *rl, read_node *n) {
  read_node *tmp = rl->head;

  if (tmp == NULL) {
    rl->head = n;
    rl->tail = n;
    return;
  }

  while (tmp->next != NULL) {
    tmp = tmp->next;
  }

  tmp->next = n;
  rl->tail = n;
}

void insert_read_list_into_pf_list(pf_list *pl, read_list *rl) {
  read_list *tmp = pl->head;

  if (tmp == NULL) {
    pl->head = rl;
    pl->tail = rl;
    return;
  }

  while (tmp->next != NULL) {
    tmp = tmp->next;
  }

  tmp->next = rl;
  pl->tail = rl;
}

bool is_trigger_duplicated(pf_list *pl, size_t md, void *bp_offset) {
  read_list *tmp = pl->head;
  
  if (tmp == NULL) {
    perror("is_trigger_duplicated");
    return -1;
  }
  
  while (1) {
    if (tmp->md == md && tmp->bp_offset == bp_offset) {
      return true;
    }
    if (tmp->next == NULL) {
      return false;
    }
    tmp = tmp->next;
  }
}


trigger_list *init_trigger_list() {
  trigger_list *newlist = (trigger_list *)malloc(sizeof(trigger_list));

  newlist->head = NULL;
  newlist->tail = NULL;

  return newlist;
}

trigger_t *new_trigger_node(struct hashmap_element_s * const e) {
  trigger_t *newnode = (trigger_t *)malloc(sizeof(trigger_t));

  newnode->ret_addr = (void *)strtol(e->key, 0, 16);
  newnode->ts = (long long)e->data;
  newnode->next = NULL;

  return newnode;
}

void insert_trigger_node(trigger_list *tl, trigger_t *tn) {
  trigger_t *t = tl->head;
  trigger_t *tmp;

  if (t == NULL) {
    tl->head = tn;
    tl->tail = tn;
    return;
  }

  while (t->next != NULL) {
    if (t->ts < tn->ts) {
      t = t->next;
      continue;
    }
    
    if (t->ts > tn->ts) {
      break;
    }
  }

  if (t->ts < tn->ts) {
    if (t->next == NULL) {
      tl->tail = tn;
    }
    t->next = tn;
    return;
  }

  if (t->ts > tn->ts) {
    tmp = (trigger_t *)malloc(sizeof(trigger_t));

    memcpy(tmp, t, sizeof(trigger_t));
    memcpy(t, tn, sizeof(trigger_t));

    t->next = tmp;

    if (tmp->next == NULL) {
      tl->tail = tmp;
    }

    free(tn);
  }
}

/*
static int iterate_print(void *const context, struct hashmap_element_s* const e) {
  printf("key: %s, value: %lld\n", e->key, (long long)e->data);
  
  return 0;
}
*/

#endif /* ANALYZER_HEADER_INCLUDED */
