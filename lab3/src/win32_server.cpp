
#include <Windows.h>

#include <stdio.h>


int main(void)
{
	HANDLE pipe_handle = CreateNamedPipeA(TEXT("\\\\.\\pipe\\lab3_pipe"), PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE|PIPE_WAIT, 1, 1024, 1024, 0, 0);
	if (pipe_handle != INVALID_HANDLE_VALUE)
	{
		HANDLE server_semaphore = CreateSemaphoreA(0, 0, 1, "lab3_server_semaphore");
		if (server_semaphore != ERROR_INVALID_HANDLE)
		{
			BOOL connected = ConnectNamedPipe(pipe_handle, 0) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
			if (connected)
			{
				char input_buffer[512] = {};

				for (;;)
				{
					// sleep on server_semaphore
					WaitForSingleObject(server_semaphore, INFINITE);

					int msg_len = scanf("%512s", input_buffer);
					if (msg_len > 0)
					{
						// send data
						// signal client_semaphore
					}
					else
					{
						FlushFileBuffers(pipe_handle);
						DisconnectNamedPipe(pipe_handle);
						break;
					}
				}
			}
		}

		CloseHandle(server_semaphore);
		CloseHandle(pipe_handle);
	}

	return (0);
}
