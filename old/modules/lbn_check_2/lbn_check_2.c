#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
//#include <linux/blk_types.h>
//#include <stdlib.h>

#define procfs_name "lbn_check_2"


extern long evaluator_lba_1;
static struct proc_dir_entry *my_proc = NULL;
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

	evaluator_lba_1 = (long)lba;
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

static const struct file_operations my_proc_fops = {
	.owner = THIS_MODULE,
	.open = my_open,
	.read = my_read,
	.write = my_write,
	.release = my_release
};

int __init my_init(void)
{
	if (register_chrdev(250, "v_device", &my_proc_fops) < 0)
		printk(KERN_ALERT "driver init failed\n");
	else
		printk(KERN_ALERT "driver init successful\n");

	my_proc = proc_create(procfs_name, 0, NULL, &my_proc_fops);

	if (my_proc == NULL) {
		return -ENOMEM;
	}

	buffer = (char *)kmalloc(1024, GFP_KERNEL);

	if(buffer != NULL)
		memset(buffer, '\0', 1024);

	printk(KERN_INFO "/proc/lbn_check_2 created\n");
	return 0;
}

void __exit my_exit(void)
{
	unregister_chrdev(250, "v_device");
	printk(KERN_ALERT "driver exit successful\n");

	remove_proc_entry(procfs_name, NULL);
	kfree(buffer);
}

module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");


