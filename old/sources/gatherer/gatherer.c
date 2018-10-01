#define	_GNU_SOURCE

#include <linux/fs.h>
#include <sys/ioctl.h>
#include <linux/fiemap.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <libgen.h>
#include "structure.h"

#define	LBN_START	929523712
#define	SECTOR_SIZE	512

mmap_list *newMList() {
	mmap_list *newlist = (mmap_list*)malloc(sizeof(mmap_list));
	
	return newlist;
}

/* Get a section header of ELF binary's text section */
int getTextsecHdr(Ehdr *elfHdr, Shdr *sectHdr, char *filename) {
	int idx;
	char *name = NULL;
	char buf[8];	
	FILE *fptr = fopen(filename, "rb");

	fread(buf, 1, sizeof(buf), fptr);
	rewind(fptr);

	if (buf[4] == '\001') {
		elfHdr->elfHdr_32 = malloc(sizeof(Elf32_Ehdr));
		sectHdr->sectHdr_32 = malloc(sizeof(Elf32_Shdr));
		
		fread(elfHdr->elfHdr_32, 1, sizeof(Elf32_Ehdr), fptr);

		fseek(fptr, elfHdr->elfHdr_32->e_shoff + elfHdr->elfHdr_32->e_shstrndx * sizeof(Elf32_Shdr), SEEK_SET);
		fread(sectHdr->sectHdr_32, 1, sizeof(Elf32_Shdr), fptr);

		name = (char *)malloc(sectHdr->sectHdr_32->sh_size * sizeof(char));

		fseek(fptr, sectHdr->sectHdr_32->sh_offset, SEEK_SET);
		fread(name, 1, sectHdr->sectHdr_32->sh_size, fptr);

		for (idx = 0; idx < elfHdr->elfHdr_32->e_shnum; idx++) {
			fseek(fptr, elfHdr->elfHdr_32->e_shoff + idx * sizeof(Elf32_Shdr), SEEK_SET);
			fread(sectHdr->sectHdr_32, 1, sizeof(Elf32_Shdr), fptr);
			if (strncmp(".text", name + sectHdr->sectHdr_32->sh_name, 6) == 0) {
				//regs->rax = 0x20000000;
				//regs->rsi = sectHdr->sectHdr_32->sh_size + (sectHdr->sectHdr_32->sh_addr) - regs->rax;
				fclose(fptr);
				free(elfHdr->elfHdr_32);
				return 1;
			}
		}		
	}
	else if (buf[4] == '\002') {
		elfHdr->elfHdr_64 = malloc(sizeof(Elf64_Ehdr));
		sectHdr->sectHdr_64 = malloc(sizeof(Elf64_Shdr));
		
		fread(elfHdr->elfHdr_64, 1, sizeof(Elf64_Ehdr), fptr);

		fseek(fptr, elfHdr->elfHdr_64->e_shoff + elfHdr->elfHdr_64->e_shstrndx * sizeof(Elf64_Shdr), SEEK_SET);
		fread(sectHdr->sectHdr_64, 1, sizeof(Elf64_Shdr), fptr);

		name = (char *)malloc(sectHdr->sectHdr_64->sh_size * sizeof(char));

		fseek(fptr, sectHdr->sectHdr_64->sh_offset, SEEK_SET);
		fread(name, 1, sectHdr->sectHdr_64->sh_size, fptr);

		for (idx = 0; idx < elfHdr->elfHdr_64->e_shnum; idx++) {
			fseek(fptr, elfHdr->elfHdr_64->e_shoff + idx * sizeof(Elf64_Shdr), SEEK_SET);
			fread(sectHdr->sectHdr_64, 1, sizeof(Elf64_Shdr), fptr);
			if (strncmp(".text", name + sectHdr->sectHdr_64->sh_name, 6) == 0) {
				//regs->rax = 0x400000;
				//regs->rsi = sectHdr->sectHdr_64->sh_size + (sectHdr->sectHdr_64->sh_addr) - regs->rax;
				fclose(fptr);
				free(elfHdr->elfHdr_64);
				return 0;
			}
		}
	}	
	fclose(fptr);
	return -1;
}

/* create mmap sequences */
void create_mmap_seq(struct user_regs_struct *regs, char *fname, mmap_list *m_list, mmap_node *m_node, long long timestamp)
{
	int md = hash(fname);	// get message digest from the file path using has function
	
	m_node = newMNode(md, regs, timestamp, fname);	// create mmap sequence node
	appendMNode(m_list, m_node);	// append mmap node to mmap list
	//printf("\nmmap called at 0x%lx, md = %d, fname = %s, ret_mmap = 0x%lx ~ 0x%lx, timestamp = %ld", regs->rip, md, fname, regs->rax, regs->rax + regs->rsi, timestamp);
}

/* Define a new mmap node
 * @key - message distance
 * @regs->rax - mmap return value
 * @regs->rsi - mmap length
 * @regs->r9 - mmap offset
 * @timestamp - timestamp called mmap system call
 * @path - absolute path of mmaped file
 */
mmap_node *newMNode(int key, struct user_regs_struct *regs, long long timestamp, char *path)
{
	int path_len;
	long lba;

	mmap_node *newnode = (mmap_node*)malloc(sizeof(mmap_node));
	
	newnode->md = key;
	newnode->timestamp = timestamp;
	newnode->next = NULL;
	newnode->prev = NULL;


	if ((lba = get_block_address(path, regs->r9)) < 0) {
		free(newnode);
		return NULL;
	}

	newnode->mmap_start = (void *)regs->rax;
	newnode->mmap_end = (void *)(regs->rax + regs->rsi);
	newnode->mmap_len = regs->rsi;
	newnode->mmap_off = regs->r9;
	newnode->lba = lba;
	strncpy(newnode->path, path, sizeof(newnode->path));
	
	return newnode;
}

long get_block_address(char *path, unsigned long offset)
{
	int fd;
	int blocksize;
	char *fiebuf;
	struct fiemap *fiemap;
	int i;
	long lba;

	uint count = 32;

	fd = open(path, O_RDONLY | O_NONBLOCK);
	if (fd > 0) {	
		fiebuf = (char *)malloc(sizeof(struct fiemap) + (count * sizeof(struct fiemap_extent)));
		if (!fiebuf) {
			perror("Could not allocate fiemap buffers");
			return -1;
		}

		fiemap = (struct fiemap *)fiebuf;
		fiemap->fm_start = 0;
		fiemap->fm_length = ~0ULL;
		fiemap->fm_flags = 0;
		fiemap->fm_extent_count = count;
		fiemap->fm_mapped_extents = 0;

		if (ioctl(fd, FIGETBSZ, &blocksize) < 0) {
			printf("Can't get block size : %s\n", path);
			free(fiebuf);
			return -1;
		}

		if (ioctl(fd, FS_IOC_FIEMAP, (unsigned long)fiemap) < 0) {
			/*
			if (errno == EBADF)
				printf("fd is not a valid descriptor : %s\n", path);
			else if (errno == EFAULT)
				printf("argp references an inaccessible memory area : %s\n", path);
			else if (errno == EINVAL)
				printf("request or argp is not valid : %s\n", path);
			else if (errno == ENOTTY)
				printf("fd is not associated with a character special device : %s\n", path);
			else
				printf("ioctl failed with unknown error : %s\n", path);	
			*/
			free(fiebuf);
			return -1;
		}
		else {
			/*
			for (i = 0; i < fiemap->fm_mapped_extents; i++) {
				if ((fiemap->fm_extents[i].fe_logical <= offset) && ((fiemap->fm_extents[i].fe_logical + fiemap->fm_extents[i].fe_length) >= offset)) {
					lba = (fiemap->fm_extents[i].fe_physical / SECTOR_SIZE) + LBN_START;
					close(fd);
					free(fiebuf);
					return lba;
				}
			}
			*/
			lba = (fiemap->fm_extents[0].fe_physical / SECTOR_SIZE) + LBN_START;
			close(fd);
			free(fiebuf);
			return lba;
		}
	}

	return -1;
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

/* Create sequence files for breakpoint and prefetching
 *
 * @path - application's file name
 * @fp - file pointer
 * @logpath - path of the file 
 */
FILE *createSeqFiles(char *apppath, FILE *fp, char *logpath)
{
	char fname[512];

	memset(fname, '\0', 512 * sizeof(char));	
	strcpy(fname, logpath);
	strcat(fname, basename(apppath));

	if ((fp = fopen(fname, "w+")) == NULL) {
		perror("fopen");
		return NULL;
	}
	
	return fp;
}

/* Get an absolute file path from proc file system */
int getFilepath(pid_t pid, int fd, char *filename) {
	char *buf = (char *)malloc(512 * sizeof(char));
	sprintf(buf, "/proc/%d/fd/%d", pid, fd);
	memset(filename, '\0', 512 * sizeof(char));
	if (readlink(buf, filename, 512 * sizeof(char)) < 0) {
		perror("readlink");
		return -1;
	}
	return 0;

}

void create_sequence(mmap_list* list, FILE *fp_mm_seq)
{
	mmap_node *temp = list->head;

	while (temp != NULL) {
		fprintf(fp_mm_seq, "%d,%s,%lld,%lld,%p,%p,%lld,%ld\n", temp->md, temp->path, temp->mmap_off, temp->mmap_len, temp->mmap_start, temp->mmap_end, temp->timestamp, temp->lba);
		temp = temp->next;
	}
}

long long get_timestamp()
{
	struct timespec ts;
	
	if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0)
		perror("clock_gettime\n");

	return (long long) (ts.tv_sec * pow(10, 9) + ts.tv_nsec);
}

int hash(char* input) {
    int length = strlen(input);
    int hval = 0;
    h_set hset;
    hset.num = 0;
    int i, j;
    int remain = length % 4;
    for (i = 0; i < (length - remain); i += 4) {
        for (j = 0; j < 4; j++) {
            hset.str[j] = input[i + j];
        }
        hval ^= hset.num;
    }
    if (remain != 0) {
        h_set temp_set;
        temp_set.num = 0;
        for (j = 0; j < remain; j++) {
            temp_set.str[j] = input[i + j];
        }
        hval ^= temp_set.num;
    }
    return hval;
}
