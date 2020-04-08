
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

// NOTE(max): set by sig_int to false

void sigusr1_handler(int sign, siginfo_t *info, void *ptr)
{
	printf("LOLOLOOOLO\n");
	return;
}

int main(void)
{
	// set proper signal mask for SIGUSR1

	char msg[256];
	snprintf(msg, 256, "hello from %d\n", getpid());	

	sigset_t set1;
	sigemptyset(&set1);
	sigaddset(&set1, SIGUSR1);

	struct sigaction sa1 = {};
	sa1.sa_sigaction = sigusr1_handler;
	sa1.sa_flags = SA_SIGINFO;
	sigaction(SIGUSR1, &sa1, 0);	

	int res;
	sigwait(&set1, &res);	

	while (true)
	{
		char *c = msg;
		while (*c)
		{
			putchar(*c++);
		}	
		kill(getppid(), SIGUSR1);

		int sign;
		sigwait(&set1, &sign);	
		sleep(1);
	}


	return (0);
}
