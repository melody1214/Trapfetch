#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
//#include <linux/blk_types.h>
//#include <stdlib.h>

#define procfs_name "lba_check"

#define P_DIR	"trapfetch"
#define	P_FILE	"lbn_checker"

static struct proc_dir_entry *proc_dir = NULL;
static struct proc_dir_entry *proc_file = NULL;

extern long evaluator_lba;
static char *buffer = NULL;
static int buf_count;
static int lba;

static ssize_t my_read(struct file *fp, char __user *buf, size_t len, loff_t *off)
{

	printk(KERN_ALERT "my_read function called\n");
	return len;
}

static ssize_t my_write(struct file *fp, const char __user *buf, size_t len, loff_t *off)
{
	strcpy(buffer, buf);

	lba = 0;

	for (buf_count = 0; buf_count < len-1; buf_count++)
		lba = (lba<<3)+(lba<<1)+buf[buf_count]-'0';

	evaluator_lba = (long)lba;
	printk(KERN_ALERT "my_write function %ld called : %d, buffer count : %d\n", len, lba, buf_count);
	return len;
}

static int my_open(struct inode *ino, struct file *fp)
{
	printk(KERN_ALERT "my_open function called\n");
	return 0;
}

static int my_release(struct inode *ino, struct file *fp)
{
	printk(KERN_ALERT "my_release function called\n");
	return 0;
}

static const struct proc_ops my_proc_fops = {
	.proc_open = my_open,
	.proc_read = my_read,
	.proc_write = my_write,
	.proc_release = my_release
};

int __init proc_init(void)
{
	if ((proc_dir = proc_mkdir(P_DIR, NULL)) == NULL) {
		printk(KERN_ERR "driver init failed\n");
		return -1;
	}
	
	if ((proc_file = proc_create(P_FILE, 0666, proc_dir, &my_proc_fops)) == NULL) {
		printk(KERN_ERR "Unable to create /proc/%s/%s\n", P_DIR, P_FILE);
		remove_proc_entry(P_DIR, NULL);
		return -1;
	}

	buffer = (char *)kmalloc(1024, GFP_KERNEL);

	if(buffer != NULL)
		memset(buffer, '\0', 1024);

	printk(KERN_INFO "/proc/%s/%s has been created\n", P_DIR, P_FILE);
	return 0;
}

void __exit proc_exit(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
	remove_proc_entry(P_FILE, proc_dir);
	remove_proc_entry(P_DIR, NULL);
#else
	remove_proc_subtree(P_DIR, NULL);
#endif

	proc_remove(proc_file);
	proc_remove(proc_dir);

	printk(KERN_INFO "/proc/%s/%s has been removed\n", P_DIR, P_FILE);

	kfree(buffer);
}

module_init(proc_init);
module_exit(proc_exit);
MODULE_LICENSE("GPL");


