#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/kallsyms.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/fcntl.h>
#include <linux/time.h>
#include <linux/stat.h>
#include <linux/cred.h>
#include <linux/dcache.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/kthread.h>

MODULE_LICENSE("GPL");

void **sys_call_table;

void EnablePageWriting(void)
{
	write_cr0(read_cr0() & (~0x10000));
}
void DisablePageWriting(void)
{
	write_cr0(read_cr0() | 0x10000);
}

asmlinkage int (*original_open)(const char *, int, mode_t);
asmlinkage int new_open(const char *pathname, int flags, mode_t mode)
{
	printk(KERN_ALERT "OPEN : %s\n", pathname);
	return (*original_open)(pathname, flags, mode);
}


asmlinkage ssize_t (*original_read) (int, void *, size_t);
asmlinkage ssize_t new_read(int fd, void *buf, size_t nbytes)
{
	return (*original_read)(fd, buf, nbytes);
}

asmlinkage ssize_t (*original_write) (int fd, void *buf, size_t n);
asmlinkage ssize_t new_write(int fd, void *buf, size_t n)
{
	return (*original_write)(fd, buf, n);
}
*/


asmlinkage int (*original_creat) (const char *, mode_t);
asmlinkage int new_creat(const char *file, mode_t mode)
{
	return (*original_creat)(file, mode);
}


asmlinkage int (*original_rename)(const char *, const char *);
asmlinkage int new_rename(const char *oldpath, const char *newpath)
{
	printk(KERN_ALERT "RENAME : %s -> %s\n", oldpath, newpath);
	return (*original_rename)(oldpath, newpath);
}

asmlinkage int (*original_unlink)(const char *);
asmlinkage int new_unlink(const char *pathname)
{
	return (*original_unlink)(pathname);
}


asmlinkage int (*original_close) (int fd);
asmlinkage int new_close(int fd)
{
	return (*original_close)(fd);
}

static void disable_page_protection(void)
{
	unsigned long value;
	asm volatile("mov %%cr0,%0"
				 : "=r"(value));
	if (value & 0x00010000)
	{
		value &= ~0x00010000;
		asm volatile("mov %0,%%cr0"
					 :
					 : "r"(value));
	}
}

static void enable_page_protection(void)
{
	unsigned long value;
	asm volatile("mov %%cr0,%0"
				 : "=r"(value));
	if (!(value & 0x00010000))
	{
		value |= 0x00010000;
		asm volatile("mov %0,%%cr0"
					 :
					 : "r"(value));
	}
}

static int __init init_hello(void)
{

	sys_call_table = kallsyms_lookup_name("sys_call_table"); // returned address of sys_call_table

	disable_page_protection(); // enable to write the sys_call_table's address area
	//EnablePageWriting();
	{
		original_open = sys_call_table[__NR_open];
		//original_read = sys_call_table[__NR_read];
		//original_write = sys_call_table[__NR_write];
		//original_creat = sys_call_table[__NR_creat];
		original_rename = sys_call_table[__NR_rename];
		original_unlink = sys_call_table[__NR_unlink];
		//original_close = sys_call_table[__NR_close];
		sys_call_table[__NR_open] = new_open;
		//sys_call_table[__NR_read] = new_read;
		//sys_call_table[__NR_write] = new_write;
		//sys_call_table[__NR_creat] = new_creat;
		sys_call_table[__NR_rename] = new_rename;
		sys_call_table[__NR_unlink] = new_unlink;
		//sys_call_table[__NR_close] = new_close;
	}
	//DisablePageWriting();
	enable_page_protection(); // disable to write the sys_call_table's address area

	printk(KERN_ALERT "MODULE INSERTED\n");

	return 0;
}

static void __exit exit_hello(void)
{

	disable_page_protection(); // enable to write the sys_call_table's address area
	//EnablePageWriting();
	{
		sys_call_table[__NR_open] = original_open;
		//sys_call_table[__NR_read] = original_read;
		//sys_call_table[__NR_write] = original_write;
		//sys_call_table[__NR_creat] = original_creat;
		sys_call_table[__NR_rename] = original_rename;
		sys_call_table[__NR_unlink] = original_unlink;
		//sys_call_table[__NR_close] = original_close;
	}
	//DisablePageWriting();
	enable_page_protection(); // disable to write the sys_call_table's address area

	printk(KERN_ALERT "MODULE REMOVED\n");
}

module_init(init_hello);
module_exit(exit_hello);