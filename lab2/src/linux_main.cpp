
#include <termios.h>
#include <stdio.h>
#include <unistd.h>

struct termios old, current;

void init_termios() 
{
	tcgetattr(0, &old); 
	current = old;
	current.c_lflag &= ~ICANON; /* disable buffered i/o */
	current.c_lflag &= ~ECHO; /* set no echo mode */ 
	tcsetattr(0, TCSANOW, &current); 
}

void reset_termios(void) 
{
	tcsetattr(0, TCSANOW, &old);
}

void fork_ps()
{
	if (fork())
	{
		// parent
	}
	else
	{
		char *const args[] = {
			(char *const)"linux_child",
			0
		};

		if (execv("./linux_child", args) == (-1))
		{
			printf("ERROR :: can't execute process");
		}
	}
}


int main(void) {
	init_termios();

	fork_ps();

	reset_termios(); 
	return 0;
}
