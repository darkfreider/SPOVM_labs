
#include <Windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
	HANDLE mutex = OpenSemaphoreA(EVENT_ALL_ACCESS, 0, "lab3_mutex");
	if (!mutex)
	{
		exit(1);
	}

	HANDLE str_present = OpenSemaphoreA(EVENT_ALL_ACCESS, 0, "lab3_str_present");
	if (!str_present)
	{
		CloseHandle(mutex);
		exit(1);
	}

	HANDLE str_not_present = OpenSemaphoreA(EVENT_ALL_ACCESS, 0, "lab3_str_not_present");
	if (!str_not_present)
	{
		CloseHandle(mutex);
		CloseHandle(str_present);
		exit(1);
	}

	HANDLE file_mapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "FMAP");
	if (file_mapping)
	{
		char *shared_data_buffer = (char *)MapViewOfFile(file_mapping, FILE_MAP_ALL_ACCESS, 0, 0, 1024);
		if (shared_data_buffer)
		{
			for (;;)
			{
				WaitForSingleObject(str_present, INFINITE);
				WaitForSingleObject(mutex, INFINITE);

				int len = strlen(shared_data_buffer);
				printf("%d: %s", len, shared_data_buffer);

				ReleaseSemaphore(mutex, 1, 0);
				ReleaseSemaphore(str_not_present, 1, 0);
			}

			UnmapViewOfFile(shared_data_buffer);
		}

		CloseHandle(file_mapping);
	}


	CloseHandle(mutex);
	CloseHandle(str_present);
	CloseHandle(str_not_present);

	return (0);
}
