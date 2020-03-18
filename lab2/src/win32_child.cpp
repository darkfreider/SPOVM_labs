
#include <string.h>
#include <Windows.h>
#include <processthreadsapi.h>

#include <stdio.h>


int main(void)
{
	char msg[256] = {};
	snprintf(msg, 256, "hello from %d\n", GetCurrentProcessId());

	HANDLE e = OpenEventA(EVENT_ALL_ACCESS, FALSE, "GLOBAL_SYNCH_EVENT");

	for (;;)
	{
		DWORD r = WAIT_FAILED;
		while (r != WAIT_OBJECT_0)
		{
			r = WaitForSingleObject(e, INFINITE);
		}

		char *c = msg;
		while (*c)
		{
			putchar(*c++);
		}

		SetEvent(e);

		Sleep(500);
	}

	return (0);
}