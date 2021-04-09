#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
//#include <linux/blk_types.h>
//#include <stdlib.h>

#define DATA_SIZE 1024
#define PROC_DIR	"trapfetch"
#define	PROC_ENTRY	"lbn_checker"

extern long evaluator_lba;

static struct proc_dir_entry *proc_dir;
static struct proc_dir_entry *proc_file;

static char *buffer;
static int buf_count;
static int lba;

static ssize_t my_read(struct file *fp, char __user *buf, size_t len, loff_t *off)
{
	int err;
	char *data = PDE_DATA(file_inode(fp));

	if (!(data)) {
		printk(KERN_INFO "[lbn_checker] read data is NULL");
		return 0;
	}

	if (len == 0) {
		printk(KERN_INFO "[lbn_checker] read of size zero, doing nothing.");
		return len;
	}

	err = copy_to_user(buf, data, len);
	printk(KERN_INFO "[lbn_checker] logical block %s has been accessed to read", buf);
	
	return len;
}

static ssize_t my_write(struct file *fp, const char __user *buf, size_t len, loff_t *off)
{
	/*
	int i;
	char *data = PDE_DATA(file_inode(fp));

	if (len > DATA_SIZE) {
		return -EFAULT;
	}

	if (copy_from_user(data, buf, len)) {
		return -EFAULT;
	}

	data[len - 1] = '\0';
	*/

	strcpy(buffer, buf);

	lba = 0;

	for (buf_count = 0; buf_count < len-1; buf_count++)
		lba = (lba<<3)+(lba<<1)+buf[buf_count]-'0';

	evaluator_lba = (long)lba;
	printk(KERN_ALERT "my_write function %ld called : %d, buffer count : %d", len, lba, buf_count);
	return len;
}

static int my_open(struct inode *ino, struct file *fp)
{
	printk(KERN_ALERT "my_open function called");
	return 0;
}

static int my_release(struct inode *ino, struct file *fp)
{
	printk(KERN_ALERT "my_release function called");
	return 0;
}

static const struct proc_ops my_proc_fops = {
	.proc_open = my_open,
	.proc_read = my_read,
	.proc_write = my_write,
	.proc_release = my_release
};

int proc_init(void)
{
	if ((proc_dir = proc_mkdir(PROC_DIR, NULL)) == NULL) {
		printk(KERN_ERR "driver init failed\n");
		return -ENOMEM;
	}

	buffer = (char *)kmalloc(DATA_SIZE, GFP_KERNEL);

	if(buffer != NULL)
		memset(buffer, '\0', 1024);


	if ((proc_file = proc_create_data(PROC_ENTRY, 0666, proc_dir, &my_proc_fops, buffer)) == NULL) {
		printk(KERN_ALERT "Error: Could not initialize /proc/%s/%s\n", PROC_DIR, PROC_ENTRY);
		remove_proc_entry(PROC_DIR, NULL);
		return -ENOMEM;
	}

	printk(KERN_INFO "/proc/%s/%s has been created\n", PROC_DIR, PROC_ENTRY);
	return 0;
}

void proc_exit(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0)
	remove_proc_entry(PROC_ENTRY, proc_dir);
	remove_proc_entry(PROC_DIR, NULL);
#else
	remove_proc_subtree(PROC_DIR, NULL);
#endif

	proc_remove(proc_file);
	proc_remove(proc_dir);

	printk(KERN_INFO "/proc/%s/%s has been removed\n", PROC_DIR, PROC_ENTRY);

	kfree(buffer);
}

module_init(proc_init);
module_exit(proc_exit);
MODULE_LICENSE("GPL");


