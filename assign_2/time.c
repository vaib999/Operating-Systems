#include <linux/posix-timers.h>

asmlinkage int my_xtime(struct timespec *current_time)
{
	struct timespec temp;
	if(access_ok(VERIFY_WRITE,current_time,sizeof(current_time)))
	{
		*current_time = current_kernel_time();
		temp = *current_time;
		printk("Time in nanoseconds: %lu\n",temp.tv_nsec);
		return temp.tv_nsec;
	}
	else
	{
		printk("Invalid address\n");
		return ENOMEM;
	}
}
