
#include <Windows.h>
#include <stdio.h>


int main(void)
{
	HANDLE pipe_handle = CreateFileA(TEXT("\\\\.\\pipe\\lab3_pipe"), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (pipe_handle != INVALID_HANDLE_VALUE)
	{
		HANDLE client_semaphore = CreateSemaphoreA(0, 0, 1, "lab3_client_semaphore");
		if (client_semaphore != 0)
		{
			char input_buffer[512] = {};

			for (;;)
			{
				HANDLE server_semaphore = OpenSemaphoreA(SYNCHRONIZE, FALSE, "lab3_server_semaphore");
				ReleaseSemaphore(server_semaphore, 1, 0);
				CloseHandle(server_semaphore);

				printf("waiting on a client semaphore\n");
				WaitForSingleObject(client_semaphore, INFINITE);

				DWORD bytes_read;
				int string_size = 0;
				ReadFile(pipe_handle, &string_size, sizeof(string_size), &bytes_read, 0);
				ReadFile(pipe_handle, input_buffer, string_size, &bytes_read, 0);
				input_buffer[string_size] = 0;
				printf("%s", input_buffer);
			}

	
		}
	}


	return (0);
}
