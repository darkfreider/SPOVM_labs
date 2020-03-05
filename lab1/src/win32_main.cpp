
#include "app.h"

#if defined(GENERATE_PARENT_APP)
	#include "parent_app.cpp"
#elif defined(GENERATE_CHILD_APP)
	#include "child_app.cpp"
#else
	#error "specify target to generate!"
#endif


#include <string.h>
#include <Windows.h>
#include <processthreadsapi.h>


PLATFORM_PROCESS_CREATE(win32_platform_process_create)
{
	static char buffer[1024];
	buffer[0] = 0;
	strcat(buffer, module_name);
	strcat(buffer, ".exe");

	PROCESS_INFORMATION process_info = {};
	STARTUPINFOA startup_info = {};
	startup_info.cb = sizeof(startup_info);


	int result = CreateProcessA(
		buffer, 0, 0, 0, 0, FALSE, 0, 0,
		&startup_info,
		&process_info);

	return (result);
}

PLATFORM_CONSOLE_WRITE(win32_platform_console_write)
{
	DWORD n;
	WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), buffer, length, &n, 0);
}

PLATFORM_PROCESS_SLEEP(win32_platform_process_sleep)
{
	Sleep(seconds * 1000);
}

PLATFORM_GET_TIME(win32_platform_get_time)
{
	App_time_info result = {};

	SYSTEMTIME time = {};
	GetLocalTime(&time);

	result.hour = (int)time.wHour;
	result.minute = (int)time.wMinute;
	result.second = (int)time.wSecond;

	return (result);
}


int WINAPI WinMain(
	HINSTANCE instance,
	HINSTANCE prev_instance,
	LPSTR     cmd_line,
	int       show_cmd)
{
	if (AllocConsole())
	{
		Platform_callbacks callbacks = {};
		callbacks.platform_process_create = win32_platform_process_create;
		callbacks.platform_console_write = win32_platform_console_write;
		callbacks.platform_process_sleep = win32_platform_process_sleep;
		callbacks.platform_get_time = win32_platform_get_time;

		application_main(&callbacks);
	}

	return (0);
}