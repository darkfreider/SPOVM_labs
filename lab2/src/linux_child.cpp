
#include <stdio.h>
#include <unistd.h>

int main(void)
{
	for (;;)
	{
		printf("hello from %d\n", getpid());
		sleep(1);
	}	

	return (0);
}
