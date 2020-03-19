
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

// NOTE(max): set by sig_int to false


int main(void)
{
	// set proper signal mask for SIGUSR1

	char msg[256];
	snprintf(msg, 256, "hello from %d\n", getpid());	

	sigset_t set;
	sigemptyset(&set);
	if (sigaddset(&set, SIGUSR1) == (-1))
	{
		printf("ERROR :: can't add signalg to a set\n");
		exit(1);
	}
	
	while (true)
	{
		int received_signal;
		//sigwait(&set, &received_signal);
		//printf("%d\n", received_signal);

		char *c = msg;
		while (*c)
		{
			//putchar(*c++);
			printf("%c", *c++);
		}
	
		// send signal to the parent process

		//usleep(250000);
		//exit(0);
	}


	return (0);
}
