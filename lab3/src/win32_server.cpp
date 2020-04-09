
#include <Windows.h>

#include <stdio.h>


int main(void)
{
	HANDLE pipe_handle = CreateNamedPipeA(TEXT("\\\\.\\pipe\\lab3_pipe"), PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE|PIPE_WAIT, 1, 1024, 1024, 0, 0);
	if (pipe_handle != INVALID_HANDLE_VALUE)
	{
		HANDLE server_semaphore = CreateSemaphoreA(0, 0, 1, "lab3_server_semaphore");
		if (server_semaphore != 0)
		{
			printf("waiting for a client to connect\n");
			BOOL connected = ConnectNamedPipe(pipe_handle, 0) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
			if (connected)
			{
				char input_buffer[512] = {};

				for (;;)
				{
					printf("waiting on a server semaphore\n");
					WaitForSingleObject(server_semaphore, INFINITE);

					printf("enter data: ");
					int msg_len = scanf("%512s", input_buffer);
					if (msg_len > 0)
					{
						DWORD bytes_written = 0;
						WriteFile(pipe_handle, input_buffer, msg_len, &bytes_written, 0);

						HANDLE client_semaphore = OpenSemaphoreA(SYNCHRONIZE, FALSE, "lab3_client_semaphore");
						ReleaseSemaphore(client_semaphore, 1, 0);
						CloseHandle(client_semaphore);
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
