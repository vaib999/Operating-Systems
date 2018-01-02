#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

void main(int argc, char *argv[])
{
	int num_read;
	int fd;

	if( argc != 1) 
	{
		printf("NUMBER OF ARGUMENTS WRONG");
		exit(1);
	}

	if ( (fd = open("/dev/my_misc_device",O_RDWR)) < 0) 
	{
		perror(""); printf("error in opening file");
		exit(1);
	}

	while(1) 	
	{
		// read a line
		ssize_t ret = read(fd, &num_read, sizeof(num_read));
		if( ret > 0) 
		{
			printf("Number read: %d ", num_read);
			printf("Bytes read: %ld\n", ret);
		} 
		else 
		{
			fprintf(stderr, "error reading ret=%ld errno=%d perror: ", ret, errno);
			perror("");
			sleep(1);
		}
	}

	close(fd);
	return;
}

