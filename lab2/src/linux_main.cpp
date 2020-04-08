#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <assert.h>
#include <sys/wait.h>

#define MAX(x, y) ((x) >= (y) ? (x) : (y))

typedef struct BufHdr
{
        size_t len;
        size_t cap;
        char buf[0];
} BufHdr;

#define buf__hdr(b) ((BufHdr *)((char *)b - offsetof(BufHdr, buf)))

#define buf__fits(b, n) (buf_len(b) + (n) <= buf_cap(b))
#define buf__fit(b, n) (buf__fits(b, n) ? 0 : ((*(void **)&(b)) = buf__grow((b), buf_len(b) + (n), sizeof(*(b)))))

#define buf_len(b) ((b) ? buf__hdr(b)->len : 0)
#define buf_cap(b) ((b) ? buf__hdr(b)->cap : 0)
#define buf_push(b, x) (buf__fit(b, 1), (b)[buf_len(b)] = (x), buf__hdr(b)->len++)
#define buf_pop(b) ((b)[--(buf__hdr(b)->len)])

// TODO(max): test this macro in buf_test
#define buf_free(b) ((b) ? free(buf__hdr(b)), (void)((b)=NULL) : (void)0)

void *buf__grow(const void *buf, size_t new_len, size_t elem_size)
{
        size_t new_cap = MAX(1 + 2 * buf_cap(buf), new_len);
        assert(new_len <= new_cap);

        size_t new_size = offsetof(BufHdr, buf) + new_cap * elem_size;
        BufHdr *new_hdr;
        if (buf)
        {
                new_hdr = (BufHdr *)realloc(buf__hdr(buf), new_size);
        }
        else
        {
                new_hdr = (BufHdr *)malloc(new_size);
                new_hdr->len = 0;
        }
        new_hdr->cap = new_cap;

        return new_hdr->buf;
}

void buf_test(void)
{
        int *s = NULL;
        enum { N = 1024 };

        for (int i = 0; i < N; i++)
        {
                buf_push(s, i);
        }
        assert(buf_len(s) == N);

        for (int i = 0; i < N; i++)
        {
                assert(s[i] == i);
        }

        buf_free(s);
        assert(s == NULL);
}


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


size_t volatile curr_ps = 0;
pid_t *volatile pbuf = 0;

// process scheduler
void sigusr1_handler_parent(int)
{
	//printf("schedule\n");
	//assert(buf_len(pbuf) != 0);
	
	if (buf_len(pbuf) > 0)
	{
		//printf("choosing process to wake up (in parent\n");
		size_t next_ps = (curr_ps) % buf_len(pbuf);
		curr_ps++;
		kill(pbuf[next_ps], SIGUSR1);
	}
}
void sigusr2_handler_parent(int)
{
	//printf("sigusr2_parent\n");
	return;
}

bool volatile wake_up = false;
void sigusr1_handler_child(int)
{
	//printf("sigusr_child\n");
	wake_up = true;
}

bool volatile g_exit_child = false;
void sigterm_handler_child(int)
{
	//printf("sigterm_child\n");
	//g_exit_child = true;
	exit(0);	
}

pid_t fork_ps()
{
	pid_t result = fork();
	
	if (!result)
	{
		struct sigaction new_sigusr1 = {};
		new_sigusr1.sa_handler = sigusr1_handler_child;
		sigemptyset(&new_sigusr1.sa_mask);
		new_sigusr1.sa_flags = 0;
		sigaction(SIGUSR1, &new_sigusr1, 0);
	
		struct sigaction new_sigterm = {};
		new_sigterm.sa_handler = sigterm_handler_child;
		sigemptyset(&new_sigterm.sa_mask);
		new_sigterm.sa_flags = 0;
		sigaction(SIGTERM, &new_sigterm, 0);

		//printf("child process has been launched\n");
		//signal(SIGUSR1, sigusr1_handler_child); 
		//signal(SIGTERM, sigterm_handler_child);

		sigset_t set;
		sigemptyset(&set);
		sigaddset(&set, SIGUSR1);
		
		kill(getppid(), SIGUSR2);

		for (;;)
		{
			//printf("start waiting (child)\n");
			
			sigset_t new_mask, old_mask;
			sigemptyset(&new_mask);
			sigemptyset(&old_mask);

			sigaddset(&new_mask, SIGUSR1);
			sigaddset(&new_mask, SIGTERM);
			sigprocmask(SIG_BLOCK, &new_mask, &old_mask);
			
			// TODO(max): mask all signals (critical sectioin)
			if (wake_up)
			{
				wake_up = false;	
				//printf("after wakeup (child)\n");	
				if (!g_exit_child)
				{
					sleep(1);
					printf("child %d\n", getpid());
					kill(getppid(), SIGUSR1);
					//printf("after yeilding (child)\n");
				}
				else
				{
					exit(0);
				}
			}	
			
			sigprocmask(SIG_SETMASK, &old_mask, 0);
				
		}

	}
	else if (result == -1)
	{
		printf("ERROR :: can't fork a process\n");
		exit(1);
	}

	//printf("about to pause and wait for init of a child\n");
	pause();
	//printf("after wakeup (parent)\n");

	return (result);
}


int main(void) {
	buf_test();
	
	init_termios();
	
	struct sigaction new_sigusr1 = {};
	new_sigusr1.sa_handler = sigusr1_handler_parent;
	sigemptyset(&new_sigusr1.sa_mask);
	new_sigusr1.sa_flags = 0;
	sigaction(SIGUSR1, &new_sigusr1, 0);

	struct sigaction new_sigusr2 = {};
	new_sigusr2.sa_handler = sigusr2_handler_parent;
	sigemptyset(&new_sigusr2.sa_mask);
	new_sigusr2.sa_flags = 0;
	sigaction(SIGUSR2, &new_sigusr2, 0);
		
	//signal(SIGUSR1, sigusr1_handler_parent);
	//signal(SIGUSR2, sigusr2_handler_parent);

	bool running = true;
	while (running)
	{
		char c = getchar();
		if (c == 'q')
		{
			printf("terminating\n");
			
			sigset_t new_mask, old_mask;
			sigemptyset(&new_mask);
			sigemptyset(&old_mask);

			sigaddset(&new_mask, SIGUSR1);
			sigprocmask(SIG_BLOCK, &new_mask, &old_mask);
			
			running = false;
			for (size_t i = 0; i < buf_len(pbuf); i++)
			{
				kill(pbuf[i], SIGTERM);
			}

			for (size_t i = 0; i < buf_len(pbuf); i++)
			{
				int s;
				waitpid(pbuf[i], &s, 0);
			}

			sigprocmask(SIG_SETMASK, &old_mask, 0);
			reset_termios();
			exit(0);
		}
		else if (c == '+')
		{
			// TODO(max): need to mask signals
			//printf("main +\n");
			pid_t p = fork_ps();
			//printf("before + cricical section\n");

			sigset_t new_mask, old_mask;
			sigemptyset(&new_mask);
			sigemptyset(&old_mask);

			sigaddset(&new_mask, SIGUSR1);
			sigprocmask(SIG_BLOCK, &new_mask, &old_mask);

			buf_push(pbuf, p);
			//printf("pushing child pd (parent)\n");
			if (buf_len(pbuf) == 1)
			{
				//printf("raising!\n");
				kill(pbuf[0], SIGUSR1);
			}
			sigprocmask(SIG_SETMASK, &old_mask, 0);
		}
		else if (c == '-')
		{
			// TODO(max): need to mask signals
			sigset_t new_mask, old_mask;
			sigemptyset(&new_mask);
			sigemptyset(&old_mask);

			sigaddset(&new_mask, SIGUSR1);
			sigprocmask(SIG_BLOCK, &new_mask, &old_mask);
			
			
			if (buf_len(pbuf) > 0)
			{
				pid_t last_ps = buf_pop(pbuf);
				kill(last_ps, SIGTERM);
				kill(last_ps, SIGUSR1);
			}

			sigprocmask(SIG_SETMASK, &old_mask, 0);
		}
	}

	reset_termios(); 
	return 0;
}












