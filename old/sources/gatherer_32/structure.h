#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
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

struct user_regs_struct
{
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
};

typedef struct _mmap_node {
	int md;
	char *path;
	long lba;
	void *mmap_start;
	void *mmap_end;
	long long mmap_len;
	long long mmap_off;
	long long timestamp;
	struct _mmap_node *next;
	struct _mmap_node *prev;
} mmap_node;

typedef struct _mmap_list {
	mmap_node *head;
	mmap_node *tail;
} mmap_list;

typedef struct _pid_stack {
	struct _pid_stack_node* top;
} pid_stack;

typedef struct _pid_stack_node {
	pid_t pid;
	struct _pid_stack_node* next;
} pid_stack_node;

typedef union {
	Elf64_Ehdr *elfHdr_64;
	Elf32_Ehdr *elfHdr_32;
} Ehdr;

typedef union {
	Elf64_Shdr *sectHdr_64;
	Elf32_Shdr *sectHdr_32;
} Shdr;

typedef union {
    int num;
    char str[4];
} h_set;

//void createSeqFiles(char *path, FILE *fp_bpseq, FILE *fp_pflist_r, FILE *fp_plist_m);
FILE *createSeqFiles(char *apppath, FILE *fp_bpseq, char *logpath);

mmap_list *newMList();

mmap_node *newMNode(int key, struct user_regs_struct *regs, long long timestamp, char *path);
int getTextsecHdr(Ehdr *elfHdr, Shdr *sectHdr, char *filename);
void create_mmap_seq(struct user_regs_struct *regs, char *fname, mmap_list *m_list, mmap_node *m_node, long long timestamp);

long get_block_address(char *path, unsigned long offset);

void appendMNode(mmap_list* list, mmap_node* node);

int getFilepath(pid_t pid, int fd, char *filename);
int getELFmagicnum(char *filename);

void create_sequence(mmap_list *m_list, FILE *fp_mm_seq);


pid_stack* newPidStack();
void push(pid_stack* stack, pid_t pid);
pid_t getStackpid(pid_stack* stack, pid_t pid);
pid_t pop(pid_stack* stack);

long long get_timestamp();
int hash(char *fname);
