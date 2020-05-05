#include <assert.h>
#include <string.h>
#include <Windows.h>
#include <winuser.h>
#include <processthreadsapi.h>

#include <stdio.h>
#include <conio.h>


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


#define THREAD_PROC(name) DWORD WINAPI name(LPVOID arg)

CRITICAL_SECTION g_terminal_lock;
int *thread_need_to_exit;

THREAD_PROC(thread_print_id)
{
	int exit_status_index = *((int *)arg);

	DWORD tid = GetCurrentThreadId();
	

	char msg[256] = {};
	snprintf(msg, 256, "hello from %d\n", tid);

	bool running = true;
	while (running)
	{
		EnterCriticalSection(&g_terminal_lock);

		char *c = msg;
		while (*c)
		{
			putchar(*c++);	
		}
		
		running = !thread_need_to_exit[exit_status_index];

		LeaveCriticalSection(&g_terminal_lock);

		Sleep(1000);
	}

	return (0);
}

struct Thread_info
{
	HANDLE t;
	int param;
};

Thread_info *create_thread(int param)
{
	Thread_info *result = (Thread_info *)malloc(sizeof(Thread_info));

	result->param = param;
	result->t = CreateThread(0, 0, thread_print_id, &result->param, 0, 0);

	return (result);	
}

void free_thread(Thread_info *info)
{
	if (info)
	{
		free(info);
	}
}

int main(void)
{
	buf_test();
	InitializeCriticalSection(&g_terminal_lock);

	// NOTE(max): dynamic array of thread ids
	Thread_info **t_buf = 0;

	bool running = true;
	while (running)
	{
		char c = getch();
		if (c == 'q')
		{
			running = false;

			EnterCriticalSection(&g_terminal_lock);
			for (size_t i = 0; i < buf_len(t_buf); i++)
			{	
				thread_need_to_exit[t_buf[i]->param] = 1;
			}
			LeaveCriticalSection(&g_terminal_lock);

			for (size_t i = 0; i < buf_len(t_buf); i++)
			{
				WaitForSingleObject(t_buf[i]->t, INFINITE);
			}
		}
		else if (c == '+')
		{
			EnterCriticalSection(&g_terminal_lock);

			Thread_info *info = create_thread(buf_len(t_buf));
			buf_push(t_buf, info);
			buf_push(thread_need_to_exit, 0);

			LeaveCriticalSection(&g_terminal_lock);
		}
		else if (c == '-')
		{
			EnterCriticalSection(&g_terminal_lock);
			if (buf_len(t_buf) > 0)
			{
				Thread_info *info = buf_pop(t_buf);
				thread_need_to_exit[info->param] = 1;

				free_thread(info);
				buf_pop(thread_need_to_exit);	
			}
			LeaveCriticalSection(&g_terminal_lock);
		}
	}

	DeleteCriticalSection(&g_terminal_lock);
	
	return (0);
}







































