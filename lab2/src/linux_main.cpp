#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

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

pid_t fork_ps()
{
	pid_t result = fork();
	if (result)
	{
		// parent
	}
	else if (!result)
	{
		char *const args[] = {
			(char *const)"linux_child",
			0
		};

		if (execv("./linux_child", args) == (-1))
		{
			printf("ERROR :: can't execute process\n");
		}
	}
	else
	{
		printf("ERROR :: can't fork a process\n");
	}

	return (result);
}

size_t curr_ps = 0;
pid_t *pbuf = 0;

int main(void) {
	buf_test();
	init_termios();
	bool running = true;
	while (running)
	{
		char c = getchar();
		if (c == 'q')
		{
			running = false;
			for (size_t i = 0; i < buf_len(pbuf); i++)
			{
				kill(pbuf[i], SIGTERM);
			}
			// terminate all prosesses with kill(pid, SIG_INT);
		}
		else if (c == '+')
		{
			pid_t p = fork_ps();
			buf_push(pbuf, p);
		}
		else if (c == '-')
		{
			/*
			pid_t last_ps = buf_pop(pbuf);
			terminate last_ps
			*/
		}
	}
	//fork_ps();

	reset_termios(); 
	return 0;
}












