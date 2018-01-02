#include<stdio.h>
#include<unistd.h>
#include<time.h>

int main()
{	
	struct timespec current_time;

	syscall(326,&current_time);
	printf("Time in nanoseconds: %lu\n",current_time.tv_nsec);
	return 0;
}

