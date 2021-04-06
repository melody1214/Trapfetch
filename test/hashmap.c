#include "../analyzer/hashmap.h"
#include <stdio.h>

#define OPEN_READ 0
#define OPEN_WRITE 1

#define PATH_READ_LOG "/home/melody/work/trapfetch/logs/r."
#define PATH_CANDIDATE_LOG "/home/melody/work/trapfetch/logs/c."

typedef struct {
  char key[64];
  long long ts;
} hashmap_t;

typedef struct trigger_node {
  void *ret_addr;
  long long ts;
  struct trigger_node *next;
} trigger_t;

typedef struct {
  trigger_t *head;
  trigger_t *tail;
} trigger_list;

struct hashmap_s hashmap;
const unsigned int map_initial_size = 2;

trigger_list *tl;

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

FILE *get_fd(char *path, char *dir, int flag) {
  char fname[512];
  FILE *fp;

  memset(fname, '\0', 512 * sizeof(char));
  strcpy(fname, dir);
  strcat(fname, path);

  if (flag == OPEN_READ) {
    if ((fp = fopen(fname, "r")) == NULL) {
      perror("fopen");
      return NULL;
    }
  } else {
    if ((fp = fopen(fname, "w+")) == NULL) {
      perror("fopen");
      return NULL;
    }
  }

  return fp;
}


static int iterate_print(void *const context, struct hashmap_element_s* const e) {
  printf("key: %s, value: %lld\n", e->key, (long long)e->data);
  
  return 0;
}

static int iterates_with_insert_trigger(void * context, struct hashmap_element_s* const e) {
  trigger_t *tn;
  tn = new_trigger_node(e);
  insert_trigger_node(tl, tn);

  return 0;
}

static int iterates(void * context, struct hashmap_element_s* const e) {
  printf("key: %s, value: %lld\n", e->key, (long long)e->data);
  if (e->data == 0) {
    return -1;
  }

  return 0;
}

int main(int argc, char *argv[]) {
    char buf[512];
    FILE *fp_fcandidates;
    hashmap_t *map_t;
    void *element;
    trigger_t *trg_t;
    
    tl = init_trigger_list();

    fp_fcandidates = get_fd(argv[1], PATH_CANDIDATE_LOG, OPEN_READ);

    if (0 != hashmap_create(map_initial_size, &hashmap)) {
        perror("hashmap_create");
        exit(EXIT_FAILURE);
        // error!
    }

    while (fgets(buf, sizeof(buf), fp_fcandidates)) {
        map_t = (hashmap_t *)malloc(sizeof(hashmap_t));
        sscanf(buf, "%[^,],%lld", map_t->key, &map_t->ts);
        
        if ((element = hashmap_get(&hashmap, map_t->key, strlen(map_t->key))) == NULL) {
          hashmap_put(&hashmap, map_t->key, strlen(map_t->key), (void *)map_t->ts);
        } else{
          hashmap_put(&hashmap, map_t->key, strlen(map_t->key), (void *)0);
        }
    }

    printf("******** hashmap scan start *********\n");
    hashmap_iterate_pairs(&hashmap, iterates, NULL);
    printf("******** hashmap scan end *********\n");
    printf("******** hashmap scan start *********\n");
    hashmap_iterate_pairs(&hashmap, iterates_with_insert_trigger, NULL);
    printf("******** hashmap scan end *********\n");
    printf("********* trigger list ***********\n");
      
    trg_t = tl->head;
    while (trg_t != NULL) {
      printf("%p, %lld\n", trg_t->ret_addr, trg_t->ts);
      trg_t = trg_t->next;
    }

    return 0;
}