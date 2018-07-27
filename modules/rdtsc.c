#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/slab.h>

#define	procfs_name	"rdtsc"

static struct proc_dir_entry *my_proc = NULL;
static char *buffer = NULL;
static int buf_count;
static int data;



static int proc_rdtsc_open(struct inode *inode, struct file *file)
{
	return single_rdtsc_open(file, write_tsc, NULL);
}

static const struct file_operations proc_rdtsc_fops = {
	.open = proc_rdtsc_open,
	.read = seq_read,
	.write = seq_write,
	.release = single_release,
};

int __init proc_rdtsc_init(void)
{
	proc_create("rdtsc", 0, NULL, &proc_rdtsc_fops);

	return 0;
}
