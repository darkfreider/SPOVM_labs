
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


int main(void)
{
	buf_test();

	PROCESS_INFORMATION *pinfo = 0;
	HANDLE *events = 0;

	// NOTE(max): auto-reset, initially in signaled state
	HANDLE global_synch_event = CreateEventA(0, FALSE, TRUE, "GLOBAL_SYNCH_EVENT");

	bool running = true;
	while (running)
	{
		char c = getch();
		if (c == 'q')
		{
			running = false;
			for (size_t i = 0; i < buf_len(pinfo); i++)
			{
				if (!TerminateProcess(pinfo[i].hProcess, 0))
				{
					printf("Can't terminate process!\n");
				}
			}
		}
		else if (c == '+')
		{
			PROCESS_INFORMATION process_info = {};
			STARTUPINFOA startup_info = {};
			startup_info.cb = sizeof(startup_info);
			if (CreateProcessA("win32_child.exe", 0, 0, 0, FALSE, NORMAL_PRIORITY_CLASS, 0, 0, &startup_info, &process_info))
			{
				buf_push(pinfo, process_info);
			}
		}
		else if (c == '-')
		{
			PROCESS_INFORMATION last_ps = buf_pop(pinfo);
			TerminateProcess(last_ps.hProcess, 0);
			CloseHandle(last_ps.hProcess);
		}
	}

	CloseHandle(global_synch_event);
	
	return (0);
}
