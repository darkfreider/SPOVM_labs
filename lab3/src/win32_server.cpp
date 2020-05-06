
#include <Windows.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int main(void)
{
	
	HANDLE mutex = CreateSemaphoreA(0, 1, 1, "lab3_mutex");
	if (GetLastError() == ERROR_INVALID_HANDLE)
	{
		exit(1);
	}

	HANDLE str_present = CreateSemaphoreA(0, 0, 1, "lab3_str_present");
	if (GetLastError() == ERROR_INVALID_HANDLE)
	{
		CloseHandle(mutex);
		exit(1);
	}

	HANDLE str_not_present = CreateSemaphoreA(0, 1, 1, "lab3_str_not_present");
	if (GetLastError() == ERROR_INVALID_HANDLE)
	{
		CloseHandle(mutex);
		CloseHandle(str_present);
		exit(1);
	}

	HANDLE file_mapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 1024, "FMAP");
	if (file_mapping)
	{
		char *shared_data_buffer = (char *)MapViewOfFile(file_mapping, FILE_MAP_ALL_ACCESS, 0, 0, 1024);
		if (shared_data_buffer)
		{
			PROCESS_INFORMATION process_info = {};
			STARTUPINFOA startup_info = {};
			startup_info.cb = sizeof(startup_info);
			if (CreateProcessA("win32_client.exe", 0, 0, 0, FALSE, NORMAL_PRIORITY_CLASS, 0, 0, &startup_info, &process_info))
			{
				char text_buffer[512];
				for (;;)
				{
					char *res = fgets(text_buffer, sizeof(text_buffer), stdin);
					if (!res)
					{
						break;
					}

					WaitForSingleObject(str_not_present, INFINITE);
					WaitForSingleObject(mutex, INFINITE);

					CopyMemory(shared_data_buffer, text_buffer, 512);

					ReleaseSemaphore(mutex, 1, 0);
					ReleaseSemaphore(str_present, 1, 0);
				}
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
