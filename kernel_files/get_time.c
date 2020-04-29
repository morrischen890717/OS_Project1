#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/timer.h>

asmlinkage void sys_get_time(struct timespec *ts){
	getnstimeofday(ts);
	return;
}
