#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/printk.h>

asmlinkage void sys_my_printk(int pid, long int start_sec, long int start_nsec, long int end_sec, long int end_nsec){
	printk(KERN_INFO "[Project1] %d %ld.%09ld %ld.%09ld\n", pid, start_sec, start_nsec, end_sec, end_nsec);
	return;
}
