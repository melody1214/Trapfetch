#include "analyzer.h"
#include "file.h"
#include "queue.h"

FILE *fp_read;
FILE *fp_candidates;

queue q;
read_list *rl;
mm_list *ml;
pf_list *pl;

// an_init prepares to begin to analyze traced data.
// if it fails, returns true, if not, returns false.
bool an_init(char **argv) {
  // open log files.
  fp_read = get_fd(argv[1], PATH_READ_LOG, OPEN_READ);
  fp_candidates = get_fd(argv[1], PATH_CANDIDATE_LOG, OPEN_READ);

  if (fp_read == NULL || fp_candidates == NULL) {
    return false;
  }

  // initialize read list.
  rl = init_read_list();
  // initialize mmap list.
  ml = init_mm_list();
  // initialize prefetch group list.
  pl = init_pf_list();
  // initialize queue.
  queue_init(&q);

  return true;
}

bool analyze() {
  char buf[512];
  mm_node *m;
  read_node *r;
  struct stat st;

  bool is_last = false;

  // read line from read and candidate logs.
  if (!fgets(buf, sizeof(buf), fp_read)) {
    // end of analysis.
    is_last = true;
    goto full;
  }

  if (buf[0] == 'm') {
    if (buf[2] == 'e') {
      // Create mmap node and insert that into mmap list.
      m = new_mmap_node(buf);
      m->md = gen_message_digest(m->path);
      insert_node_into_mm_list(ml, m);
    }

    // Create read node for prefetching and enqueue.
    r = new_read_node(buf, MMAP);
  } else if (buf[0] == 'r') {
    // Create read node for prefetching and enqueue.
    r = new_read_node(buf, READ);
  }

  // If a file has not logical block address, it will not be queued.
  r->lba = get_logical_blk_addr(r->path);
  if (r->lba == 0) {
    return true;
  }

  if (stat(r->path, &st) < 0) {
    perror("stat");
    return true;
  }

  r->ino = st.st_ino;
  enqueue(&q, r);

  // Queue is not full.
  if (!is_full(&q)) {
    return true;
  }

full:
  // Queue is full. Queued data will be inserted to a read list.
  while (q.count > 0) {
    r = new_read_node(NULL, 0);
    memcpy(r, dequeue(&q), sizeof(read_node));

    insert_node_into_read_list(rl, r);
  }

  // Queue must have been reset.
  if (!is_empty(&q)) {
    perror("is_empty()");
    exit(EXIT_FAILURE);
  }

  rl->start_ts = rl->head->ts;
  rl->end_ts = rl->tail->ts;

  // The time for fully queueing is less than burst threshold.
  if (is_burst(&q)) {
    rl->is_burst = IS_BURST;
  }
  // The time for fully queueing exceeds the threshold.
  else {
    rl->is_burst = !IS_BURST;
  }

  if (is_last) {
    pl->tail = rl;
    return false;
  }

  insert_read_list_into_pf_list(pl, rl);

  rl->next = init_read_list();
  rl = rl->next;

  return true;
}

void reordering_pf_list() {
  read_list *pf;
  read_list *read;
  read_list *node_to_free;

  if (pl->head == NULL) {
    perror("reordering");
    exit(EXIT_FAILURE);
  }

  if (pl->head->next == NULL) {
    perror("reordering");
    exit(EXIT_FAILURE);
  }

  pf = pl->head;
  read = pl->head->next;
  node_to_free = NULL;

  while (read != NULL) {
    if (pf->is_burst != IS_BURST) {
      node_to_free = pf;
      pf = pf->next;
      read = read->next;
      pl->head = pf;
      free(node_to_free);
      continue;
    }

    while (read != NULL && read->is_burst == IS_BURST) {
      pf->tail->next = read->head;
      pf->tail = read->tail;
      pf->end_ts = read->end_ts;
      node_to_free = read;
      read = read->next;
      free(node_to_free);
      pf->next = read;
    }

    while (read != NULL && read->is_burst != IS_BURST) {
      node_to_free = read;
      read = read->next;
      free(node_to_free);
      pf->next = read;
    }

    if (read == NULL) {
      pl->tail = pf;
      break;
    }

    pf = pf->next;
    read = read->next;
  }
}

void swap(read_node *a, read_node *b) {
  read_node *tmp = (read_node *)malloc(sizeof(read_node));

  memcpy(tmp, b, sizeof(read_node));
  memcpy(b, a, sizeof(read_node));
  memcpy(a, tmp, sizeof(read_node));

  a->next = b;
  b->next = tmp->next;

  free(tmp);
}

// for sorting read_list into logical block address
bool reordering_read_list() {
  int swapped;
  read_list *rlist = pl->head;
  read_node *n = rlist->head;

  while (rlist != NULL) {
    do {
      swapped = 0;
      n = rlist->head;

      while (n->next != NULL) {
        if (n->lba > n->next->lba) {
          swap(n, n->next);
          swapped = 1;
        } else if (n->lba == n->next->lba) {
          if (n->off > n->next->off) {
            swap(n, n->next);
            swapped = 1;
          }
        }
        n = n->next;
      }
    } while (swapped);

    rlist = rlist->next;
    printf("reordering read_list...\n");
  }
  return swapped;
}

bool merge(read_list *rlist, read_node *a, read_node *b) {
  if (OFFTOLEN(a) < b->off) {
    return false;
  }

  if (a->off > OFFTOLEN(b)) {
    return false;
  }

  if (SUBSET(b, a)) {
    a->off = b->off;
    a->len = b->len;
  } else if (EXPAND(a, b)) {
    a->len = (b->off - a->off) + OFFTOLEN(b);
  } else if (EXPAND(b, a)) {
    a->off = b->off;
    a->len = (a->off - b->off) + OFFTOLEN(a);
  }

  if (b->next == NULL) {
    a->next = NULL;
    rlist->tail = a;
  } else {
    a->next = b->next;
  }
  free(b);
  return true;
}

bool merging_read_list() {
  read_list *rlist = pl->head;
  read_node *n = rlist->head;

  int merged;

  while (rlist != NULL) {
    do {
      merged = 0;
      n = rlist->head;

      while (n->next != NULL) {
        if (n->lba == n->next->lba) {
          if (merge(rlist, n, n->next) == true) {
            merged = 1;
          }
        }
        if (n->next == NULL) {
          break;
        }
        n = n->next;
      }
    } while (merged);
    rlist = rlist->next;
    printf("merging read_list...\n");
  }
  return merged;
}

void generate_meta_list() {
  read_list *rlist = pl->head;
  read_node *n;
  meta_node *m = NULL;

  while (rlist != NULL) {
    n = rlist->head;
    while (n != NULL) {
      m = new_meta_node(n);
      insert_meta_node_into_read_list(rlist, m);

      n = n->next;
    }
    rlist = rlist->next;
  }
}

void set_trigger() {
  char buf[512];
  void *ret_addr;
  void *bp_offset;
  unsigned long md;
  long long ts;
  long long ts_idle_begin;
  long long ts_idle_end;
  mm_node *mnode;

  read_list *rlist = pl->head;

  rlist->bp_offset = 0;
  rlist->md = 0;

  while (rlist->next != NULL) {
    ts_idle_begin = rlist->end_ts;
    ts_idle_end = rlist->next->start_ts;

    while (fgets(buf, sizeof(buf), fp_candidates)) {
      sscanf(buf, "%p,%lld", &ret_addr, &ts);

      if (ts < ts_idle_begin) {
        continue;
      }

      if (ts > ts_idle_end) {
        break;
      }

      if (rlist->next->bp_offset != NULL) {
        break;
      }

      mnode = ml->head;

      while (mnode != NULL) {
        if (mnode->ts > ts) {
          break;
        }

        if (mnode->ts > ts_idle_end) {
          break;
        }

        if ((mnode->start_addr <= ret_addr) && (mnode->end_addr >= ret_addr)) {
          bp_offset = (void *)((long long)ret_addr - (long long)mnode->start_addr);
          md = mnode->md;
          if (is_trigger_duplicated(pl, md, bp_offset)) {
            printf("triggering...\n");
            mnode = mnode->next;
            continue;
          }
          rlist->next->md = md;
          rlist->next->bp_offset = bp_offset;
          break;
        }

        mnode = mnode->next;
      }
    }

    if (rlist->next->bp_offset == NULL) {
      rlist->end_ts = rlist->next->end_ts;
      rlist->tail->next = rlist->next->head;
      rlist->tail = rlist->next->tail;
      if (rlist->next->next != NULL)
        rlist->next = rlist->next->next;
      else
        rlist->next = NULL;

      /* to recheck
      reordering_read_list();
      merging_read_list();
      reordering_read_list();
      merging_read_list();
      */
    } else {
      rlist = rlist->next;
    }
  }
}

void generate_prefetch_data(char **argv) {
  FILE *fp_bp;
  FILE *fp_pf;
  read_node *rnode;
  meta_node *mnode;

  read_list *rlist = pl->head;

  fp_bp = get_fd(argv[1], PATH_BP, OPEN_WRITE);
  fp_pf = get_fd(argv[1], PATH_PF, OPEN_WRITE);

  while (rlist != NULL) {
    mnode = rlist->meta_head;
    rnode = rlist->head;

    if (rlist == pl->head) {
      fprintf(fp_bp, "%ld,0\n", rlist->md);
      while (mnode != NULL) {
        fprintf(fp_pf, "%ld,0,%s,0,0,0,0\n", rlist->md, mnode->ptr->path);
        mnode = mnode->next;
      }
      while (rnode != NULL) {
        fprintf(fp_pf, "%ld,0,%s,%lld,%lld,%ld,1\n", rlist->md, rnode->path,
                rnode->off, rnode->len, rnode->lba);
        rnode = rnode->next;
      }
    } else {
      while (mnode != NULL) {
        fprintf(fp_pf, "%ld,%p,%s,0,0,0,0\n", rlist->md, rlist->bp_offset,
                mnode->ptr->path);
        mnode = mnode->next;
      }
      while (rnode != NULL) {
        fprintf(fp_pf, "%ld,%p,%s,%lld,%lld,%ld,1\n", rlist->md,
                rlist->bp_offset, rnode->path, rnode->off, rnode->len,
                rnode->lba);
        rnode = rnode->next;
      }
    }
    if (rlist->next != NULL) {
      if (rlist->next->bp_offset == NULL) {
        if (rlist == pl->head) {
          rlist->next->md = 0;
          rlist->next->bp_offset = 0;
        }
        rlist->next->md = rlist->md;
        rlist->next->bp_offset = rlist->bp_offset;
      } else {
        if (rlist->bp_offset != NULL) {
          fprintf(fp_bp, "%ld,%p\n", rlist->md, rlist->bp_offset);
        }
      }
    }
    rlist = rlist->next;
  }

  fclose(fp_bp);
  fclose(fp_pf);
}
