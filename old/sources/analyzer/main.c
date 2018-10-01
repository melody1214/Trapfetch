#include <stdio.h>
#include "analyzer.h"
#include "pf.h"
#include "hash.h"

static FILE *fp_read;
static FILE *fp_mmap;
static FILE *fp_etc;
static FILE *fp_bp;
static FILE *fp_pf;

int cnt_idle;
int cnt_bp;

void prn_burst_list(FILE *fp_pf, burst_list *b_list)
{
	burst_group *b_group;
	burst_node *b_node, *temp;
	ino_node *i_node;
	b_group = b_list->group_head;

	if (b_group == NULL) {
		printf("there is no burst group!\n");
		return;
	}

	create_ino_nodes(b_group);

	while (b_group != NULL) {
		b_node = b_group->node_head;
		i_node = b_group->ino_head;

		while (i_node != NULL) {
			if (b_group == b_list->group_head) {
				fprintf(fp_pf, "%ld,0,%s,0,0,0,0\n", b_group->md, i_node->path);
			}
			else {
				fprintf(fp_pf, "%ld,%p,%s,0,0,0,0\n", b_group->md, (void *)b_group->bp_offset, i_node->path);
			}
			i_node = i_node->next;
		}

		while (b_node->next != NULL) {
			temp = b_node->next;
			if (b_node->lba == temp->lba) {
				if ((b_node->offset < temp->offset) && ((b_node->offset + b_node->length) >= temp->offset) && ((b_node->offset + b_node->length) <= (temp->offset + temp->length))) {
					b_node->length = temp->offset - b_node->offset + temp->length;
					b_node->next = temp->next;
				}
				else if ((temp->offset >= b_node->offset) && ((temp->offset + temp->length) <= (b_node->offset + b_node->length))) {
					b_node->next = temp->next;
				}
				else if ((temp->offset <= b_node->offset) && (b_node->offset <= (temp->offset + temp->length)) && ((temp->offset + temp->length) < (b_node->offset + b_node->length))) {
					b_node->offset = temp->offset;
					b_node->length = b_node->offset - temp->offset + b_node->length;
					b_node->next = temp->next;
				}
				else {
					b_node = b_node->next;
				}
			}
			else
				b_node = b_node->next;
		}

		b_node = b_group->node_head;

		while (b_node != NULL) {
			if (b_group == b_list->group_head) {
				if (b_node->lba == LBN) {
					if ((PF_TARGET_OFFSET <= b_node->offset) && ((PF_TARGET_OFFSET + PF_TARGET_LENGTH) >= (b_node->offset + b_node->length))) {
						;
					}
					else if ((PF_TARGET_OFFSET >= b_node->offset) && ((PF_TARGET_OFFSET + PF_TARGET_LENGTH) <= (b_node->offset + b_node->length))) {
						if ((long long)(PF_TARGET_OFFSET - b_node->offset) > 0)
							fprintf(fp_pf, "%ld,0,%s,%lld,%lld,%ld,1\n", b_group->md, b_node->path, b_node->offset, PF_TARGET_OFFSET-b_node->offset, b_node->lba);
						fprintf(fp_pf, "%ld,0,%s,%lld,%lld,%ld,1\n", b_group->md, b_node->path, (long long) PF_TARGET_OFFSET+PF_TARGET_LENGTH, (b_node->offset+b_node->length)-(PF_TARGET_OFFSET+PF_TARGET_LENGTH), b_node->lba);
					}
					else if ((PF_TARGET_OFFSET <= b_node->offset) && ((PF_TARGET_OFFSET + PF_TARGET_LENGTH) >= b_node->offset) && ((PF_TARGET_OFFSET + PF_TARGET_LENGTH) <= (b_node->offset + b_node->length))) {
						fprintf(fp_pf, "%ld,0,%s,%lld,%lld,%ld,1\n", b_group->md, b_node->path, (long long) PF_TARGET_OFFSET+PF_TARGET_LENGTH, (b_node->offset+b_node->length)-(PF_TARGET_OFFSET+PF_TARGET_LENGTH), b_node->lba);
					}
					else if ((PF_TARGET_OFFSET >= b_node->offset) && (PF_TARGET_OFFSET <= (b_node->offset + b_node->length)) && ((PF_TARGET_OFFSET + PF_TARGET_LENGTH) >= (b_node->offset + b_node->length))) {
						if ((long long)(PF_TARGET_OFFSET - b_node->offset) > 0)
							fprintf(fp_pf, "%ld,0,%s,%lld,%lld,%ld,1\n", b_group->md, b_node->path, b_node->offset, PF_TARGET_OFFSET - b_node->offset, b_node->lba);
					}
					else {
						fprintf(fp_pf, "%ld,0,%s,%lld,%lld,%ld,1\n", b_group->md, b_node->path, b_node->offset, b_node->length, b_node->lba);
					}
				}
				else {
						fprintf(fp_pf, "%ld,0,%s,%lld,%lld,%ld,1\n", b_group->md, b_node->path, b_node->offset, b_node->length, b_node->lba);
				}
			}
			else {
				if (b_node->lba == LBN) {
					if ((PF_TARGET_OFFSET <= b_node->offset) && ((PF_TARGET_OFFSET + PF_TARGET_LENGTH) >= (b_node->offset + b_node->length))) {
						;
					}
					else if ((PF_TARGET_OFFSET >= b_node->offset) && ((PF_TARGET_OFFSET + PF_TARGET_LENGTH) <= (b_node->offset + b_node->length))) {
						if ((long long)(PF_TARGET_OFFSET - b_node->offset) > 0)
							fprintf(fp_pf, "%ld,%p,%s,%lld,%lld,%ld,1\n", b_group->md, (void *)b_group->bp_offset, b_node->path, b_node->offset, PF_TARGET_OFFSET-b_node->offset, b_node->lba);
						fprintf(fp_pf, "%ld,%p,%s,%lld,%lld,%ld,1\n", b_group->md, (void *)b_group->bp_offset, b_node->path, (long long) PF_TARGET_OFFSET+PF_TARGET_LENGTH, (b_node->offset+b_node->length)-(PF_TARGET_OFFSET+PF_TARGET_LENGTH), b_node->lba);
					}
					else if ((PF_TARGET_OFFSET <= b_node->offset) && ((PF_TARGET_OFFSET + PF_TARGET_LENGTH) >= b_node->offset) && ((PF_TARGET_OFFSET + PF_TARGET_LENGTH) <= (b_node->offset + b_node->length))) {
						fprintf(fp_pf, "%ld,%p,%s,%lld,%lld,%ld,1\n", b_group->md, (void *)b_group->bp_offset, b_node->path, (long long) PF_TARGET_OFFSET+PF_TARGET_LENGTH, (b_node->offset+b_node->length)-(PF_TARGET_OFFSET+PF_TARGET_LENGTH), b_node->lba);
					}
					else if ((PF_TARGET_OFFSET >= b_node->offset) && (PF_TARGET_OFFSET <= (b_node->offset + b_node->length)) && ((PF_TARGET_OFFSET + PF_TARGET_LENGTH) >= (b_node->offset + b_node->length))) {
						if ((long long)(PF_TARGET_OFFSET - b_node->offset) > 0)
							fprintf(fp_pf, "%ld,%p,%s,%lld,%lld,%ld,1\n", b_group->md, (void *)b_group->bp_offset, b_node->path, b_node->offset, PF_TARGET_OFFSET - b_node->offset, b_node->lba);
					}
					else {
						fprintf(fp_pf, "%ld,%p,%s,%lld,%lld,%ld,1\n", b_group->md, (void *)b_group->bp_offset, b_node->path, b_node->offset, b_node->length, b_node->lba);
					}
				}
				else {
						fprintf(fp_pf, "%ld,%p,%s,%lld,%lld,%ld,1\n", b_group->md, (void *)b_group->bp_offset, b_node->path, b_node->offset, b_node->length, b_node->lba);
				}
			}
			b_node = b_node->next;
		}
		b_group = b_group->next;
	}

	return;
}

FILE *getfp(char *apppath, FILE *fp, char *dir, int flag)
{
	char fname[512];
	memset(fname, '\0', 512 * sizeof(char));
	strcpy(fname, dir);
	strcat(fname, apppath);

	if (flag == FOPEN_READ) {
		if ((fp = fopen(fname, "r")) == NULL) {
			perror("fopen");
			exit(-1);
		}
	}
	else {
		if ((fp = fopen(fname, "w+")) == NULL) {
			perror("fopen");
			exit(-1);
		}
	}

	return fp;
}

/**
  1. get all the data for read and mmap sequentially.
  2. compare timestamp of read data with mmap data.
  3. insert data into circular queue in ascending order by timestamp.
  4. examine whether data in circular queue is full or not.
  	- full: rear, front sets each -1 and 0.
	- not: insert.
**/
void analyze_data(char *argv[])
{
	char buf[512], fname[512];

	void *retaddr;
	long long etc_timestamp;
	mmap_list *m_list;
	mmap_node *m_node;
	read_node *r_node;
	etc_node *e_node, *temp_e_node;
	burst_list *b_list;
	burst_group *b_group;
	circ_queue *q;
	int is_mmap, is_burst;

	cnt_idle = 0;
	cnt_bp = 0;

	fp_read = getfp(argv[1], fp_read, "/home/shared/prefetch/logs/read_", FOPEN_READ);
	fp_mmap = getfp(argv[1], fp_mmap, "/home/shared/prefetch/logs/mm_", FOPEN_READ);
	fp_etc = getfp(argv[1], fp_etc, "/home/shared/prefetch/logs/etc_", FOPEN_READ);
	fp_bp = getfp(argv[1], fp_bp, "/home/shared/prefetch/log/bp_", FOPEN_WRITE);
	fp_pf = getfp(argv[1], fp_pf, "/home/shared/prefetch/log/pf_", FOPEN_WRITE);

	m_list = new_mmap_list();

	memset(buf, '\0', sizeof(buf));

	while (fgets(buf, sizeof(buf), fp_mmap)) {
		m_node = new_mmap_node(buf);
		append_mmap_node(m_list, m_node);
	}
	
	rewind(fp_mmap);

	q = init_queue();
	b_list = new_burst_list();
	b_group = b_list->group_head;

	memset(buf, '\0', sizeof(buf));

	if (fgets(buf, sizeof(buf), fp_mmap)) {
		sscanf(buf, "%ld,", &b_group->md);
		fprintf(fp_bp, "%ld,%lld\n", b_group->md, b_group->bp_offset);
	}

	rewind(fp_mmap);

	memset(buf, '\0', sizeof(buf));

	is_mmap = -1;
	is_burst = 0;

	e_node = malloc(sizeof(e_node));
	temp_e_node = NULL;
	
	while (!feof(fp_mmap) || !feof(fp_read)) {
		if (is_mmap > 0) {
			do {
				if (fgets(buf, sizeof(buf), fp_mmap)) {
					m_node = new_mmap_node(buf);
					memset(buf, '\0', sizeof(buf));
				}
			} while(m_node == NULL);
		}
		else if (is_mmap == 0) {
			do {
				if (fgets(buf, sizeof(buf), fp_read)) {
					r_node = new_read_node(buf);
					memset(buf, '\0', sizeof(buf));
				}
			} while(r_node == NULL);
		}
		else {
			do {
				if (fgets(buf, sizeof(buf), fp_mmap)) {
					m_node = new_mmap_node(buf);
					memset(buf, '\0', sizeof(buf));
				}
			} while(m_node == NULL);
			do {
				if (fgets(buf, sizeof(buf), fp_read)) {
					r_node = new_read_node(buf);
					memset(buf, '\0', sizeof(buf));
				}
			} while(r_node == NULL);
		}

		if (is_full(q)) {
			append_burst_list(q, b_group, b_list);
			
			if (is_idle(q)) {
				cnt_idle++;

				while (!feof(fp_etc)) {
					if (fgets(buf, sizeof(buf), fp_etc)) {
						sscanf(buf, "%p,%lld", &e_node->retaddr, &e_node->timestamp);
					}
					
					if ((temp_e_node = get_trigger(b_group, m_list, q, e_node)) != NULL)
						break;
				}

				if ((temp_e_node != NULL) && is_burst) {
					//allocate new burst group
					b_group->next = new_burst_group();
					b_group = b_group->next;
					b_list->group_tail = b_group;

					b_group->md = temp_e_node->md;
					b_group->bp_offset = temp_e_node->bp_offset;
					
					if (ftell(fp_bp) == 0)
						fprintf(fp_bp, "%ld,0\n", b_group->md);
					else {
						cnt_bp++;	
						fprintf(fp_bp, "%ld,%p\n", b_group->md, (void *)b_group->bp_offset);
					}
				}
				temp_e_node = NULL;
				is_burst = 0;
			}
			else
				is_burst = 1;

				
			reset_queue(q);
		}

		if (feof(fp_read)) {
			if (!feof(fp_mmap) && m_node != NULL) {
				append_queue_entry_for_mmap(m_node, q);
				is_mmap = 1;
				continue;
			}
			else
				break;
		}

		if (feof(fp_mmap)) {
			if (!feof(fp_read) && r_node != NULL) {
				append_queue_entry_for_read(r_node, q);
				is_mmap = 0;
				continue;
			}
			else
				break;
		}
		

		if (r_node->timestamp <= m_node->timestamp) {
			append_queue_entry_for_read(r_node, q);
			is_mmap = 0;
		}
		else {
			append_queue_entry_for_mmap(m_node, q);
			is_mmap = 1;
		}
	}

	append_burst_list(q, b_group, b_list);

	prn_burst_list(fp_pf, b_list);
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("usage: analyzer <application's name>\n");
		return -1;
	}

	analyze_data(argv);

	fclose(fp_mmap);
	fclose(fp_etc);
	fclose(fp_read);
	//fclose(fp_bp);
	fclose(fp_pf);
	
	printf("cnt_idle : %d, cnt_bp : %d\n", cnt_idle, cnt_bp);

	return 0;
}
