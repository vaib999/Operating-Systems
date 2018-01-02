#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define MAXLEN 100

int main(int argc, char* argv[])
{
	int count = 0;
	int num_to_write;
	char numstr[MAXLEN];
	int fd;

	if( argc != 1) {
		printf("Usage: %s <numpipe_name>\n", argv[0]);
		exit(1);
	}	


	if ( (fd = open("/dev/my_misc_device", O_WRONLY)) < 0) {
		perror(""); printf("Error: Device open failure!\n");
		exit(1);
	} 

	while(1)
	{
		bzero(numstr, MAXLEN);
		sprintf(numstr, "%d%d\n", getpid(), count++);
		num_to_write = atoi(numstr);
		printf("writing string %d ", num_to_write);

		ssize_t ret = write(fd, &num_to_write, sizeof(num_to_write));
		if(ret < 0)
		{
			fprintf(stderr, "error writing ret=%ld errno=%d perror: ", ret, errno);
			perror("");
		}
		else
		{
			printf("Number of bytes written = %ld\n", ret);
		}
		sleep(1);
	}
	close(fd);
	return 0;
}
