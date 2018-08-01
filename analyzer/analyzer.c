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

  // If a file has not logical block address, it will be not queued.
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
  if (pl->head == NULL) {
    perror("reordering");
    exit(EXIT_FAILURE);
  }

  if (pl->head->next == NULL) {
    perror("reordering");
    exit(EXIT_FAILURE);
  }

  read_list *pf = pl->head;
  read_list *read = pl->head->next;
  read_list *node_to_free = NULL;

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

void reordering_read_list() {
  read_list *rl = pl->head;
  read_node *n = rl->head;

  int swapped;

  while (rl != NULL) {
    do {
      swapped = 0;
      n = rl->head;

      while (n->next != NULL) {
        if (n->lba > n->next->lba) {
          swap(n, n->next);
          swapped = 1;
        }
        n = n->next;
      }
    } while (swapped);
    rl = rl->next;
    printf("reordering read_list...\n");
  }
}

bool merge(read_list *rl, read_node *a, read_node *b) {
  long long off_to_len_a = a->off + a->len;
  long long off_to_len_b = b->off + b->len;

  if (off_to_len_a < b->off) {
    return false;
  }

  if (a->off > off_to_len_b) {
    return false;
  }

  if (SUBSET(b, a)) {
    a->off = b->off;
    a->len = b->len;
  } else if (EXPAND(a, b)) {
    a->len = (b->off - a->off) + off_to_len_b;
  } else if (EXPAND(b, a)) {
    a->off = b->off;
    a->len = (a->off - b->off) + off_to_len_a;
  }

  if (b->next == NULL) {
    a->next = NULL;
    rl->tail = a;
  } else {
    a->next = b->next;
  }
  free(b);
  return true;
}

void merging_read_list() {
  read_list *rl = pl->head;
  read_node *n = rl->head;

  int merged;

  while (rl != NULL) {
    do {
      merged = 0;
      n = rl->head;

      while (n->next != NULL) {
        if (n->lba == n->next->lba) {
          if (merge(rl, n, n->next) == true) {
            merged = 1;
          }
        }
        if (n->next == NULL) {
          break;
        }
        n = n->next;
      }
    } while (merged);
    rl = rl->next;
    printf("merging read_list...\n");
  }
}

void generate_prefetch_data() {
  char buf[512];
  void *ret_addr;
  long long ts;

  read_list *rl = pl->head;

  if (rl->next != NULL) {
  }

  // read line from candidate logs.
  while (fgets(buf, sizeof(buf), fp_candidates)) {
    sscanf(buf, "%p,%lld", &ret_addr, &ts);
  }
}