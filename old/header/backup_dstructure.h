#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <libgen.h>
#include <wait.h>
#include <sys/syscall.h>
#include <signal.h>
#include <elf.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

typedef struct _mmap_node {
	int md;
	unsigned long long mmap_start;
	unsigned long long mmap_end;
	long timestamp;
	struct _pf_list *pf_list;
	struct _mmap_node *next;
	struct _mmap_node *prev;
} mmap_node;

typedef struct _mmap_list {
	mmap_node *head;
	mmap_node *tail;
} mmap_list;

typedef struct _pf_node {
	int ino; // inode number
	unsigned long long lba; // Logical Block Address
	unsigned long long bp_offset; // Breakpoint target offset
	char *path; // File path
	loff_t offset; // File offset
	ssize_t ret; // Read length
	struct _pf_node *next;
	struct _pf_node *prev;
} pf_node;

typedef struct _pf_list {
	pf_node *head;
	pf_node *tail;
} pf_list;

mmap_node *newMNode(int key, unsigned long long start, unsigned long long end, long timestamp) {
	mmap_node *newnode = (mmap_node*)malloc(sizeof(mmap_node));
	pf_list *new_pf_list = (pf_list *)malloc(sizeof(pf_list));
	new_pf_list->head = NULL;
	new_pf_list->tail = NULL;

	newnode->md = key;
	newnode->mmap_start = start;
	newnode->mmap_end = end;
	newnode->timestamp = timestamp;
	newnode->pf_list = new_pf_list;
	newnode->next = NULL;
	newnode->prev = NULL;
	return newnode;
}

mmap_list *newMList() {
	mmap_list *newlist = (mmap_list*)malloc(sizeof(mmap_list));
	newlist->head = NULL;
	newlist->tail = NULL;
	return newlist;
}

void appendMNode(mmap_list* list, mmap_node* node) {
	if (list->head == NULL) {
		list->head = node;
		list->tail = node;
	}
	else {
		list->tail->next = node;
		node->prev = list->tail;
		list->tail = node;
	}
}

int searchpfNode(unsigned long long lba, loff_t file_offset, ssize_t ret_read, pf_list *list) {
	pf_node *temp = list->head;
	
	if (temp == NULL)
		return 1;

	while (temp != NULL) {
		if (temp->lba == lba) {
			if (((file_offset >= temp->offset) && (file_offset <= (temp->offset + temp->ret))) && (((file_offset + ret_read) >= temp->offset) && ((file_offset + ret_read) <= (temp->offset + temp->ret))))
				return -1;
			else if (((file_offset >= temp->offset) && (file_offset <= (temp->offset + temp->ret))) && ((file_offset + ret_read) > (temp->offset + temp->ret))) {
				temp->ret = temp->ret + ((file_offset + ret_read) - (temp->offset + temp->ret));
				return 0;
			}
			else if ((file_offset < temp->offset) && ((file_offset + ret_read) > (temp->offset + temp->ret))) {
				temp->offset = file_offset;
				temp->ret = ret_read;
				return 0;
			}
			else if (((file_offset + ret_read) >= temp->offset) && ((file_offset + ret_read) <= (temp->offset + temp->ret))) {
				temp->offset = file_offset;
				return 0;
			}
		}
		temp = temp->next;
	}
	return 1;
}
		

pf_node *newpfNode(struct stat *fileStatus, unsigned long long bpOffset, char *path, unsigned long long lba, loff_t offset, ssize_t ret) {
	pf_node *newnode = (pf_node *)malloc(sizeof(pf_node));
	newnode->path = malloc(strlen(path)+1);
	strncpy(newnode->path, path, strlen(path)+1);
	newnode->ino = (int)fileStatus->st_ino;
	newnode->lba = lba;
	newnode->bp_offset = bpOffset;
	newnode->offset = offset;
	newnode->ret = ret;
	newnode->next = NULL;
	newnode->prev = NULL;
	return newnode;
}

void addpfNode(pf_list *list, pf_node *node) {
	if (list->head == NULL) {
		list->head = node;
		list->tail = node;
	}
	else {
		pf_node *temp = list->head;
		
		if (temp->bp_offset > node->bp_offset) {
			node->next = temp;
			temp->prev = node;
			list->head = node;
		}
		else if ((temp->bp_offset <= node->bp_offset) && (temp->lba <= node->lba)) {			
			if (temp == list->tail) {
				node->prev = temp;
				temp->next = node;
				list->tail = node;
			}
			else {
				while (temp != list->tail) {
					if (temp->bp_offset < node->bp_offset) {
						temp = temp->next;
					}
					else if ((temp->bp_offset == node->bp_offset) && (temp->lba <= node->lba)) {
						temp = temp->next;
					}
					else {
						break;
					}
				}
			
				if (temp == list->tail) {
					if (temp->bp_offset <= node->bp_offset) {
						node->prev = temp;
						temp->next = node;
						list->tail = node;
					}
					else {
						node->next = temp;
						node->prev = temp->prev;
						temp->prev->next = node;
						temp->prev = node;
					}
				}
				else {
					node->next = temp;
					node->prev = temp->prev;
					temp->prev->next = node;
					temp->prev = node;
				}
			}
		}		
	}
}

mmap_node *getMNode(mmap_list* list, unsigned long long address, long timestamp) {
	mmap_node* temp = list->tail;
	while (temp != NULL) {
		if (((temp->mmap_start) <= address) && (address <= (temp->mmap_end)) && (timestamp > (temp->timestamp))) {
			return temp;
		}
		temp = temp->prev;
	}
	return NULL;
}

int createSequence(mmap_list* list, FILE *bp, FILE *pf) {
	unsigned long long bpOffset;
	mmap_node *temp = list->head;
	pf_node *pf_temp = NULL;
	while (temp != NULL) {
		if (temp->pf_list->head != NULL) {
			pf_temp = temp->pf_list->head;
		}
		while (pf_temp != NULL) {
			bpOffset = pf_temp->bp_offset;
			if (pf_temp->next == NULL) {
				fprintf(bp, "%d,0x%llx\n", temp->md, bpOffset);
				fprintf(pf, "%d,0x%llx,%s,%ld,%ld\n", temp->md, pf_temp->bp_offset, pf_temp->path, pf_temp->offset, pf_temp->ret);
			}
			else {
				if (bpOffset != pf_temp->next->bp_offset)
					fprintf(bp, "%d,0x%llx\n", temp->md, bpOffset);
				fprintf(pf, "%d,0x%llx,%s,%ld,%ld\n", temp->md, pf_temp->bp_offset, pf_temp->path, pf_temp->offset, pf_temp->ret);
			}
			
			pf_temp = pf_temp->next;
		}
		if (temp == list->tail)
			return 0;
		temp = temp->next;
	}
	return -1;
}

typedef struct _pid_stack_node {
	pid_t pid;
	struct _pid_stack_node* next;
} pid_stack_node;

typedef struct _pid_stack {
	struct _pid_stack_node* top;
} pid_stack;

pid_stack* newPidStack() {
	pid_stack* newStack = (pid_stack*)malloc(sizeof(pid_stack));
	newStack->top = NULL;
	return newStack;
}

void push(pid_stack* stack, pid_t pid) {
	pid_stack_node* temp_node = (pid_stack_node*)malloc(sizeof(pid_stack_node));
	temp_node->pid = pid;
	if ( stack->top != NULL) {
		temp_node->next = stack->top;
	}
	else {
		temp_node->next = NULL;
	}
	stack->top = temp_node;
}


pid_t getStackpid(pid_stack* stack, pid_t pid) {
	pid_stack_node* temp_node = NULL;
	if (stack->top != NULL) {
		temp_node = stack->top;
		if (temp_node->next != NULL) {
			do {
				if (pid == temp_node->pid) {
					return pid;
				}
				temp_node = temp_node->next;
			} while(temp_node != NULL);
		}
		else {
			if (pid == temp_node->pid)
				return pid;
		}
	}
	return -1;
}

pid_t pop(pid_stack* stack) {
    if(stack->top == NULL){
        return -1;
    }
    pid_stack_node* temp_node = stack->top;
    stack->top = stack->top->next;
    return temp_node->pid;
}