#define	_GNU_SOURCE

#include "structure.h"
#include "hash.h"
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <linux/fiemap.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <libgen.h>

mmap_list *newMList() {
	mmap_list *newlist = (mmap_list*)malloc(sizeof(mmap_list));
	newlist->head = NULL;
	newlist->tail = NULL;
	return newlist;
}

pf_mmaped_list *init_pf_mmaped_list() {
	pf_mmaped_list *newlist = (pf_mmaped_list *)malloc(sizeof(pf_mmaped_list));
	newlist->head = NULL;
	newlist->tail = NULL;
	return newlist;
}

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

pf_mmaped_node *new_pf_mmaped_node(char *path, off_t offset, size_t length) {
	int fd;
	int blocksize;
	char *fiebuf;
	struct fiemap *fiemap;
	int i;
	long long lba;

	uint count = 32;
	fiebuf = (char *)malloc(sizeof(struct fiemap) + (count * sizeof(struct fiemap_extent)));

	pf_mmaped_node *newnode = (pf_mmaped_node *)malloc(sizeof(pf_mmaped_node));
	fd = open(path, O_RDONLY);
	if (fd > 0) {	
		if (!fiebuf) {
			perror("Could not allocate fiemap buffers");
			exit(-1);
		}

		fiemap = (struct fiemap *)fiebuf;
		fiemap->fm_start = 0;
		fiemap->fm_length = ~0ULL;
		fiemap->fm_flags = 0;
		fiemap->fm_extent_count = count;
		fiemap->fm_mapped_extents = 0;

		if (ioctl(fd, FIGETBSZ, &blocksize) < 0) {
			printf("Can't get block size : %s\n", path);
			perror("ioctl");
			exit(-1);
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
		}
		else {
			for (i = 0; i < fiemap->fm_mapped_extents; i++) {
				if ((fiemap->fm_extents[i].fe_logical <= (int)offset / blocksize) && ((fiemap->fm_extents[i].fe_logical + fiemap->fm_extents[i].fe_length) > (int)offset / blocksize)) {
					newnode->lba = (fiemap->fm_extents[i].fe_physical / blocksize);
					newnode->path = (char *)malloc(strlen(path)+1);
					newnode->path = strncpy(newnode->path, path, strlen(path)+1);
					newnode->offset = offset;
					newnode->length = length;
					newnode->next = NULL;
					close(fd);
					return newnode;
				}
			}
		}
		return NULL;
	}
	else {
		perror("open");
		return NULL;
	}
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

/* create mmap sequences
regs.rax = mmap return value (mmapping starting address), regs.rsi = mmapping length
regs.r8 = file descriptor, regs.r9 = mmapping offset */
void createMmapseq(struct user_regs_struct *regs, char *fname, mmap_list *m_list, mmap_node *m_node, long timestamp) {
	int md = hash(fname);	// get message digest from the file path using hash function
	m_node = newMNode(md, regs->rax, regs->rax + regs->rsi, timestamp);	// create mmap sequence node
	appendMNode(m_list, m_node);	// append mmap node to mmap list
	//printf("\nmmap called at 0x%llx, md = %d, fname = %s, ret_mmap = 0x%llx ~ 0x%llx, timestamp = %ld", regs->rip, md, fname, regs->rax, regs->rax + regs->rsi, timestamp);
}

/* Create read sequences
ra_read = return address of read call, lba = starting LBA of file which should be prefetched
rv_read = return value of read call */
int createReadseq(mmap_list *m_list, char *filename, struct stat *fileStatus, unsigned long long ra_read, long timestamp, unsigned long long lba, loff_t file_offset, ssize_t rv_read) {
	unsigned long long bpOffset;
	
	mmap_node *mNode = NULL;
	pf_node *pNode = NULL;

	if (lba == 0) {
		mNode = m_list->tail;
		stat(filename, fileStatus);
		pNode = newpfNode(fileStatus, 0x0, filename, lba, file_offset, rv_read);
		addpfNode(mNode->pf_list, pNode);
		return 0;
	}
	else {
		if ((mNode = getMNode(m_list, ra_read, timestamp)) == NULL) {	// get mmapped file information which include instructions of this read call
			//perror("getMNode");
			return -1;
		}

		stat(filename, fileStatus);
	
		bpOffset = ra_read - (mNode->mmap_start);	// find an offset of injecting breakpoint target address
	
		if (searchpfNode(lba, file_offset, rv_read, mNode->pf_list) == 1) {
			pNode = newpfNode(fileStatus, bpOffset, filename, lba, file_offset, rv_read); // create read sequence node
			addpfNode(mNode->pf_list, pNode); // add read node to prefetching list
			//printf("\nread called at 0x%llx, md = %d, fname = %s, bp_offset = 0x%llx, timestamp = %ld", (mNode->mmap_start) + bpOffset, mNode->md, filename, bpOffset, timestamp);
			return 0;
		}
		else
			return -1;
	}
}

/*
 * @path - file path
 * @regs.r9 - offset
 * @regs.rsi - length
 */
int create_pf_mmap_seq(char *path, struct user_regs_struct *regs, pf_mmaped_list *pf_m_list)
{
	int ret;
	pf_mmaped_node *pf_m_node = NULL;
	pf_m_node = new_pf_mmaped_node(path, regs->r9, regs->rsi);
	if (pf_m_node == NULL) {
		//printf("unavailable mmaped file : %s\n", path);
		return -1;
	}
		
	ret = add_pf_mmaped_node(pf_m_node, pf_m_list);
	if (ret < 0) {
		//printf("mmaped nodes have same LBA each other\n");
		return -1;
	}

	return 0;
}

int add_pf_mmaped_node(pf_mmaped_node *pf_m_node, pf_mmaped_list *pf_m_list)
{
	if (pf_m_list->head == NULL) {
		pf_m_list->head = pf_m_node;
		pf_m_list->tail = pf_m_node;
	}
	else {
		pf_mmaped_node *temp = pf_m_list->head;
		
		if (temp->lba > pf_m_node->lba) {
			pf_m_node->next = temp;
			pf_m_list->head = pf_m_node;
			return 0;
		}
		else if (temp->lba == pf_m_node->lba) {
			return -1;
		}
		else {
			while (temp->next != NULL) {
				if (temp->next->lba > pf_m_node->lba) {
					pf_m_node->next = temp->next;
					temp->next = pf_m_node;
					return 0;
				}
				else if (temp->next->lba == pf_m_node->lba) {
					return -1;
				}
				else {
					temp = temp->next;
				}
			}
			temp->next = pf_m_node;
			pf_m_list->tail = pf_m_node;
		}
	}
	return 0;
}


/* Create sequence files for breakpoint and prefetching
 *
 * @path - application's file name
 * @fp_bpseq - file pointer of breakpoint sequences
 * @fp_pflist_r - file pointer of prefetching sequences for read
 * @fp_pflist_m - file pointer of prefetching sequences for mmaped files
 */
FILE *createSeqFiles(char *apppath, FILE *fp, char *logpath)
{
	char fname[256];

	memset(fname, '\0', 256 * sizeof(char));	
	strcpy(fname, logpath);
	fp = fopen(strcat(fname, basename(apppath)), "w+");

	//memset(fname, '\0', 256 * sizeof(char));
	//strcpy(fname, "/home/shared/prefetch/logs/pf_read_");
	//fp_pflist_r = fopen(strcat(fname, basename(path)), "w+");

	//memset(fname, '\0', 256 * sizeof(char));
	//strcpy(fname, "/home/shared/prefetch/logs/pf_mmap_");
	//fp_pflist_m = fopen(strcat(fname, basename(path)), "w+");
	return fp;
}

/* Get an absolute file path from proc file system */
void getFilepath(pid_t pid, int fd, char *filename) {
	char *buf = (char *)malloc(256 * sizeof(char));
	sprintf(buf, "/proc/%d/fd/%d", pid, fd);
	memset(filename, '\0', 256 * sizeof(char));
	readlink(buf, filename, 256 * sizeof(char));
}

/* Get a section header of ELF binary's text section */
int getTextsecHdr(Elf64_Ehdr *elfHdr, Elf64_Shdr *sectHdr, char *filename) {
	int idx;
	char *name = NULL;
	
	FILE *fptr = fopen(filename, "rb");
	fread(elfHdr, 1, sizeof(Elf64_Ehdr), fptr);
	
	fseek(fptr, elfHdr->e_shoff + elfHdr->e_shstrndx * sizeof(Elf64_Shdr), SEEK_SET);
	fread(sectHdr, 1, sizeof(Elf64_Shdr), fptr);
	
	name = (char *)malloc(sectHdr->sh_size * sizeof(char));

	fseek(fptr, sectHdr->sh_offset, SEEK_SET);
	fread(name, 1, sectHdr->sh_size, fptr);
	
	for (idx = 0; idx < elfHdr->e_shnum; idx++) {
		fseek(fptr, elfHdr->e_shoff + idx * sizeof(Elf64_Shdr), SEEK_SET);
		fread(sectHdr, 1, sizeof(Elf64_Shdr), fptr);

		if (strncmp(".text", name + sectHdr->sh_name, 6) == 0) {
			fclose(fptr);
			return 0;
		}
	}
	fclose(fptr);
	return -1;
}

int searchpfNode(unsigned long long lba, loff_t file_offset, ssize_t rv_read, pf_list *list) {
	pf_node *temp = list->head;
	
	if (temp == NULL)
		return 1;

	while (temp != NULL) {
		if (temp->lba == lba) {
			if (((file_offset >= temp->offset) && (file_offset <= (temp->offset + temp->ret))) && (((file_offset + rv_read) >= temp->offset) && ((file_offset + rv_read) <= (temp->offset + temp->ret))))
				return -1;
			else if (((file_offset >= temp->offset) && (file_offset <= (temp->offset + temp->ret))) && ((file_offset + rv_read) > (temp->offset + temp->ret))) {
				temp->ret = temp->ret + ((file_offset + rv_read) - (temp->offset + temp->ret));
				return 0;
			}
			else if ((file_offset < temp->offset) && ((file_offset + rv_read) > (temp->offset + temp->ret))) {
				temp->offset = file_offset;
				temp->ret = rv_read;
				return 0;
			}
			else if (((file_offset + rv_read) >= temp->offset) && ((file_offset + rv_read) <= (temp->offset + temp->ret))) {
				temp->offset = file_offset;
				return 0;
			}
		}
		temp = temp->next;
	}
	return 1;
}

int createSequence(mmap_list *m_list, pf_mmaped_list *pf_m_list, FILE *bp, FILE *pf_read, FILE *pf_mmap) {
	unsigned long long bpOffset;
	mmap_node *temp = m_list->head;
	pf_mmaped_node *temp_mmaped = pf_m_list->head;
	pf_node *pf_temp = NULL;

	while (temp_mmaped != NULL) {
		fprintf(pf_mmap, "100,0x%llx,%s,%ld,%ld\n", temp_mmaped->lba, temp_mmaped->path, temp_mmaped->offset, temp_mmaped->length);
		temp_mmaped = temp_mmaped->next;
	}

	while (temp != NULL) {
		if (temp->pf_list->head != NULL) {
			pf_temp = temp->pf_list->head;
		}
		while (pf_temp != NULL) {
			bpOffset = pf_temp->bp_offset;
			if (pf_temp->next == NULL) {
				fprintf(bp, "%d,0x%llx\n", temp->md, bpOffset);
				fprintf(pf_read, "%d,0x%llx,%s,%ld,%ld\n", temp->md, pf_temp->bp_offset, pf_temp->path, pf_temp->offset, pf_temp->ret);
			}
			else {
				if (bpOffset != pf_temp->next->bp_offset)
					fprintf(bp, "%d,0x%llx\n", temp->md, bpOffset);
				fprintf(pf_read, "%d,0x%llx,%s,%ld,%ld\n", temp->md, pf_temp->bp_offset, pf_temp->path, pf_temp->offset, pf_temp->ret);
			}
			
			pf_temp = pf_temp->next;
		}
		if (temp == m_list->tail)
			return 0;
		
		temp = temp->next;
	}
	return -1;
}
