#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define _LARGEFILE64_SOURCE 1
#define	_FILE_OFFSET_BITS	64	

#define	QUEUE_MAX	128
#define	BURST_THRESHOLD	2000000000
#define	FOPEN_READ	1
#define	FOPEN_WRITE	0


typedef struct {
	char path[512];
	long long timestamp;
	long long offset;
	long long length;
	long lba;
	int ino;
}queue_entry;

typedef struct {
	int front;
	int rear;
	int count;
	queue_entry ele[QUEUE_MAX];
}circ_queue;

typedef struct _read_node{
	char path[512];
	long long timestamp;
	long long offset;
	long long length;
	long lba;
	int ino;
}read_node;

typedef struct _mmap_node{
	char path[512];
	long long timestamp;
	long long offset;
	long long length;
	long lba;
	unsigned long md;
	int ino;
	void *mm_start;
	void *mm_end;
	struct _mmap_node *next;
}mmap_node;

typedef struct {
	mmap_node *mmap_head;
	mmap_node *mmap_tail;
}mmap_list;

typedef struct _burst_node{
	char path[512];
	long long timestamp;
	long long offset;
	long long length;
	long lba;
	int ino;
	struct _burst_node *next;
}burst_node;

typedef struct _ino_node{
	int ino;
	char path[512];
	struct _ino_node *next;
}ino_node;

typedef struct _burst_group{
	long initial_time;
	unsigned long md;
	long bp_offset;
	ino_node *ino_head;
	ino_node *ino_tail;
	burst_node *node_head;
	burst_node *node_tail;
	struct _burst_group *next;
}burst_group;

typedef struct {
	burst_group *group_head;
	burst_group *group_tail;
}burst_list;

typedef struct {
	void *retaddr;
	long long timestamp;
	unsigned long md;
	long long bp_offset;
}etc_node;

void append_mmap_node(mmap_list *m_list, mmap_node *m_node)
{
	mmap_node *temp;

	if (m_list->mmap_head == NULL) {
		m_list->mmap_head = m_node;
		m_list->mmap_tail = m_node;
	}
	else {
		temp = m_list->mmap_tail;
		temp->next = m_node;
		m_list->mmap_tail = m_node;
	}

	return;
}

mmap_node *new_mmap_node(char *buf)
{
	mmap_node *newnode;
	struct stat filestat;

	char fname[512];

	memset(fname, '\0', sizeof(fname));
	
	newnode = malloc(sizeof(mmap_node));

	sscanf(buf, "%ld,%[^,],%lld,%lld,%p,%p,%lld,%ld", &newnode->md, fname, &newnode->offset, &newnode->length, &newnode->mm_start, &newnode->mm_end, &newnode->timestamp, &newnode->lba);
	strncpy(newnode->path, fname, sizeof(newnode->path));

	if (newnode->length == 0) {
		free(newnode);
		return NULL;
		//stat(fname, &filestat);
		//newnode->length = filestat.st_size - newnode->offset;
		//newnode->ino = filestat.st_ino;
	}
	else {
		stat(fname, &filestat);
		newnode->ino = filestat.st_ino;
	}

	newnode->next = NULL;

	memset(buf, '\0', sizeof(buf));
	
	return newnode;
}

read_node *new_read_node(char *buf)
{
	read_node *newnode;
	struct stat filestat;

	char fname[512];

	memset(fname, '\0', sizeof(fname));
	
	newnode = malloc(sizeof(read_node));

	sscanf(buf, "%[^,],%lld,%lld,%lld,%ld", fname, &newnode->timestamp, &newnode->offset, &newnode->length, &newnode->lba);
	strncpy(newnode->path, fname, sizeof(newnode->path));

	if (newnode->length == 0) {
		free(newnode);
		return NULL;
		//stat(fname, &filestat);
		//newnode->length = filestat.st_size - newnode->offset;
		//newnode->ino = filestat.st_ino;
	}
	else {
		stat(fname, &filestat);
		newnode->ino = filestat.st_ino;
	}

	memset(buf, '\0', sizeof(buf));
	
	return newnode;
}

mmap_list *new_mmap_list()
{
	mmap_list *newlist = malloc(sizeof(mmap_list));
	newlist->mmap_head = NULL;
	newlist->mmap_tail = NULL;
	return newlist;
}

burst_node *new_burst_node()
{
	burst_node *newnode = malloc(sizeof(burst_node));
	memset(newnode->path, '\0', sizeof(newnode->path));
	newnode->offset = 0;
	newnode->length = 0;
	newnode->lba = 0;
	newnode->ino = 0;
	newnode->next = NULL;
	return newnode;
}

ino_node *new_ino_node()
{
	ino_node *newnode = malloc(sizeof(ino_node));

	memset(newnode->path, '\0', sizeof(newnode->path));
	newnode->next = NULL;
}

burst_group *new_burst_group()
{
	burst_group *newgroup = malloc(sizeof(burst_group));
	newgroup->initial_time = 0;
	newgroup->md = 0;
	newgroup->bp_offset = 0;
	newgroup->node_head = NULL;
	newgroup->node_tail = NULL;
	newgroup->ino_head = NULL;
	newgroup->ino_tail = NULL;
	newgroup->next = NULL;
	return newgroup;
}

burst_list *new_burst_list()
{
	burst_list *newlist = malloc(sizeof(burst_list));
	burst_group *newgroup = new_burst_group();
	newlist->group_head = newgroup;
	newlist->group_tail = newgroup;
	return newlist;
}

burst_node *copy_from_queue(circ_queue *q, int index)
{
	burst_node *newnode;
	
	newnode = new_burst_node();

	strncpy(newnode->path, q->ele[index].path, sizeof(newnode->path));

	newnode->lba = q->ele[index].lba;
	newnode->timestamp = q->ele[index].timestamp;
	newnode->offset = q->ele[index].offset;
	newnode->length = q->ele[index].length;
	newnode->ino = q->ele[index].ino;
	newnode->next = NULL;
	
	return newnode;
}

etc_node *get_trigger(burst_group *b_group, mmap_list *m_list, circ_queue *q, etc_node *e_node)
{
	mmap_node *temp;
	long long q_initial_time = q->ele[q->front].timestamp;
	long long q_last_time = q->ele[q->rear].timestamp;

	temp = m_list->mmap_head;

	if (e_node->timestamp <= q_initial_time)
		return NULL;

	while (temp != NULL)
	{
		if (temp->timestamp >= e_node->timestamp) {
			break;
		}

		if ((temp->mm_start <= e_node->retaddr) && (temp->mm_end >= e_node->retaddr)) {
			e_node->md = temp->md;
			e_node->bp_offset = e_node->retaddr - temp->mm_start;
			return e_node;
		}

		temp = temp->next;
	}

	return NULL;
}

int is_idle(circ_queue *q)
{
	int idle = 0;
	long long ts_sub = q->ele[q->rear].timestamp - q->ele[q->front].timestamp;

	if (ts_sub >= 2000000000) {
		printf("idle period : %lld\n", ts_sub);
		idle = 1;
	}
	else
		printf("burst period : %lld\n", ts_sub);
	
	return idle;
}

int is_full(circ_queue *q)
{
	int full = 0;

	if (q->count == QUEUE_MAX)
		full = 1;

	return full;
}

void insert_queue(circ_queue *q, queue_entry *item)
{
	if (is_full(q)) {
		printf("\nQueue overflow\n");
		return;
	}

	q->rear = (q->rear + 1) % QUEUE_MAX;

	strncpy(q->ele[q->rear].path, item->path, sizeof(q->ele[q->rear].path));
	q->ele[q->rear].timestamp = item->timestamp;
	q->ele[q->rear].offset = item->offset;
	q->ele[q->rear].length = item->length;
	q->ele[q->rear].lba = item->lba;
	q->ele[q->rear].ino = item->ino;

	q->count++;

	free(item);
}

void reset_queue(circ_queue *q)
{
	int i;

	q->front = 0;
	q->rear = -1;
	q->count = 0;

	i = 0;

	while (i < QUEUE_MAX) {
		memset(q->ele[i].path, '\0', sizeof(q->ele[i].path));

		q->ele[i].timestamp = 0;
		q->ele[i].offset = 0;
		q->ele[i].length = 0;
		q->ele[i].lba = 0;
		q->ele[i].ino = 0;

		i++;
	}
}

circ_queue *init_queue()
{
	int i;
	circ_queue *q = malloc(sizeof(circ_queue));
	
	q->front = 0;
	q->rear = -1;
	q->count = 0;

	i = 0;
	while (i < QUEUE_MAX) {
		memset(q->ele[i].path, '\0', sizeof(q->ele[i].path));

		q->ele[i].timestamp = 0;
		q->ele[i].offset = 0;
		q->ele[i].length = 0;
		q->ele[i].lba = 0;
		q->ele[i].ino = 0;

		i++;
	}

	return q;
}



void append_queue_entry_for_mmap(mmap_node *node, circ_queue *q)
{
	queue_entry *entry;

	entry = malloc(sizeof(queue_entry));

	memset(entry->path, '\0', sizeof(entry->path));
	
	strncpy(entry->path, node->path, sizeof(entry->path));

	entry->timestamp = node->timestamp;
	entry->offset = node->offset;
	entry->length = node->length;
	entry->lba = node->lba;
	entry->ino = node->ino;

	insert_queue(q, entry);
}

void append_queue_entry_for_read(read_node *node, circ_queue *q)
{
	queue_entry *entry;

	entry = malloc(sizeof(queue_entry));

	memset(entry->path, '\0', sizeof(entry->path));
	
	strncpy(entry->path, node->path, sizeof(entry->path));

	entry->timestamp = node->timestamp;
	entry->offset = node->offset;
	entry->length = node->length;
	entry->lba = node->lba;
	entry->ino = node->ino;

	insert_queue(q, entry);
}

void append_burst_list(circ_queue *q, burst_group *b_group)
{
	burst_node *node, *node_prev, *temp;
	int i;

	if (b_group->node_head == NULL) {	
		temp = copy_from_queue(q, q->front);
		b_group->node_head = temp;
		b_group->node_tail = temp;
		b_group->initial_time = q->ele[q->front].timestamp;
		i = q->front + 1;
	}
	else {
		i = q->front;
	}

	for (; i <= q->rear; i++) {
		node_prev = NULL;
		temp = NULL;
		node = NULL;
		node = b_group->node_head;
		temp = copy_from_queue(q, i);
		
		while (node != NULL) {
			if (temp->lba < node->lba) {
				temp->next = node;
				if (node == b_group->node_head) {
					b_group->node_head = temp;
				}
				else {
					node_prev->next = temp;
				}

				break;
			}
			else if (temp->lba == node->lba) {
				if ((temp->offset < node->offset) && ((temp->offset + temp->length) >= node->offset) && ((temp->offset + temp->length) <= (node->offset + node->length))) {
					node->length = node->offset - temp->offset + node->length;
					node->offset = temp->offset;
					break;
				}
				else if ((node->offset >= temp->offset) && ((node->offset + node->length) <= (temp->offset + temp->length))) {
					node->offset = temp->offset;
					node->length = temp->length;
					break;
				}
				else if ((node->offset <= temp->offset) && (temp->offset <= (node->offset + node->length)) && ((node->offset + node->length) < (temp->offset + temp->length))) {
					node->length = temp->offset - node->offset + temp->length;
					break;
				}
				else if ((temp->offset < node->offset) && ((temp->offset + temp->length) < node->offset)) {
					temp->next = node;
					if (node == b_group->node_head) {
						b_group->node_head = temp;
					}
					else {
						node_prev->next = temp;
					}
					break;
				}
				else if ((node->offset + node->length) < temp->offset) {
					node_prev = node;
					node = node->next;
				}
				else {
					break;
				}
			}
			else {
				node_prev = node;
				node = node->next;
			}
		}
		

		if (node_prev == b_group->node_tail) {
			//node = node_prev;
			if (temp->lba > node_prev->lba) {
				node_prev->next = temp;
				b_group->node_tail = temp;
			}
			else if (temp->lba == node_prev->lba) {
				if (temp->offset > (node_prev->offset + node_prev->length)) {
					node_prev->next = temp;
					b_group->node_tail = temp;
				}
			}
		}
	}
}


void create_ino_nodes(burst_group *b_group)
{
	burst_node *b_node = NULL;
	ino_node *i_node, *i_temp, *i_prev;
	

	if (b_group == NULL) {
		printf("there is no burst group!\n");
		exit(-1);
	}

	i_node = b_group->ino_head;
	i_prev = NULL;
	while (b_group != NULL) {
		b_node = b_group->node_head;
		while (b_node != NULL) {
			if (b_group->ino_head == NULL) {
				i_node = new_ino_node();
				b_group->ino_head = i_node;
				b_group->ino_tail = i_node;
				strncpy(i_node->path, b_node->path, sizeof(b_node->path));
				i_node->ino = b_node->ino;
			}
			else {
				i_temp = b_group->ino_head;
				while (i_temp != NULL) {
					if (i_temp->ino < b_node->ino) {
						if (i_temp == b_group->ino_tail) {
							i_node = new_ino_node();
							strncpy(i_node->path, b_node->path, sizeof(b_node->path));
							i_node->ino = b_node->ino;
							i_temp->next = i_node;
							b_group->ino_tail = i_node;
							break;
						}
						else {
							i_prev = i_temp;
							i_temp = i_temp->next;
						}
					}
					else if (i_temp->ino == b_node->ino) {
						break;
					}
					else {
						i_node = new_ino_node();
						strncpy(i_node->path, b_node->path, sizeof(b_node->path));
						i_node->ino = b_node->ino;
						i_node->next = i_temp;
						if (i_temp == b_group->ino_head) {
							b_group->ino_head = i_node;
						}
						else {
							i_prev->next = i_node;
						}
						break;
					}
				}	
			}
			b_node = b_node->next;
		}
		b_group = b_group->next;
	}
}
